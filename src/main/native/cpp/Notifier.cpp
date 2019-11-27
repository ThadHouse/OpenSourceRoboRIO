/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2019 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "hal/Notifier.h"

#include <atomic>
#include <cstdlib>  // For std::atexit()
#include <memory>

#include <wpi/condition_variable.h>
#include <wpi/mutex.h>

#include "HALInitializer.h"
//#include "hal/ChipObject.h"
#include "hal/Errors.h"
#include "hal/HAL.h"
#include "hal/handles/UnlimitedHandleResource.h"

#include "NiFpga_OpenSourceRIO.h"
#include "Globals.h"
#include <thread>
#include <atomic>

using namespace hal::internal;

using namespace hal;

static constexpr int32_t kTimerInterruptNumber = 28;

static wpi::mutex notifierMutex;
static std::thread notifierThread;
static bool notifierThreadCreated = false;
static std::atomic_bool runNotifierThread;
//static std::unique_ptr<tAlarm> notifierAlarm;
//static std::unique_ptr<tInterruptManager> notifierManager;
static uint64_t closestTrigger{UINT64_MAX};

namespace {

struct Notifier {
  uint64_t triggerTime = UINT64_MAX;
  uint64_t triggeredTime = UINT64_MAX;
  bool active = true;
  wpi::mutex mutex;
  wpi::condition_variable cond;
};

}  // namespace

static std::atomic_flag notifierAtexitRegistered{ATOMIC_FLAG_INIT};
static std::atomic_int notifierRefCount{0};

using namespace hal;

class NotifierHandleContainer
    : public UnlimitedHandleResource<HAL_NotifierHandle, Notifier,
                                     HAL_HandleEnum::Notifier> {
 public:
  ~NotifierHandleContainer() {
    ForEach([](HAL_NotifierHandle handle, Notifier* notifier) {
      {
        std::scoped_lock lock(notifier->mutex);
        notifier->triggerTime = UINT64_MAX;
        notifier->triggeredTime = 0;
        notifier->active = false;
      }
      notifier->cond.notify_all();  // wake up any waiting threads
    });
  }
};

static NotifierHandleContainer* notifierHandles;

static void alarmCallback() {
  std::scoped_lock lock(notifierMutex);
  int32_t status = 0;
  uint64_t currentTime = 0;

  // the hardware disables itself after each alarm
  closestTrigger = UINT64_MAX;

  // process all notifiers
  notifierHandles->ForEach([&](HAL_NotifierHandle handle, Notifier* notifier) {
    if (notifier->triggerTime == UINT64_MAX) return;
    if (currentTime == 0) currentTime = HAL_GetFPGATime(&status);
    std::unique_lock lock(notifier->mutex);
    if (notifier->triggerTime < currentTime) {
      notifier->triggerTime = UINT64_MAX;
      notifier->triggeredTime = currentTime;
      lock.unlock();
      notifier->cond.notify_all();
    } else if (notifier->triggerTime < closestTrigger) {
      closestTrigger = notifier->triggerTime;
    }
  });

  if (closestTrigger != UINT64_MAX) {
    status = NiFpga_WriteU32(FPGASession, NiFpga_OpenSourceRIO_ControlU32_Alarm_TriggerTime, static_cast<uint32_t>(closestTrigger));
    // Simply truncate the hardware trigger time to 32-bit.
    status = NiFpga_WriteBool(FPGASession, NiFpga_OpenSourceRIO_ControlBool_Alarm_Enable, true);
    // Enable the alarm.  The hardware disables itself after each alarm.
  }
}

static void cleanupNotifierAtExit() {
  // Force cleanup
  runNotifierThread = false;
  notifierThread.join();
  //notifierAlarm = nullptr;
  //notifierManager = nullptr;
}

namespace hal {
namespace init {
void InitializeNotifier() {
  static NotifierHandleContainer nH;
  notifierHandles = &nH;
}
}  // namespace init
}  // namespace hal

extern "C" {

HAL_NotifierHandle HAL_InitializeNotifier(int32_t* status) {
  hal::init::CheckInit();
  if (!notifierAtexitRegistered.test_and_set())
    std::atexit(cleanupNotifierAtExit);

  if (notifierRefCount.fetch_add(1) == 0) {
    std::scoped_lock lock(notifierMutex);
    // create manager and alarm if not already created
    if (!notifierThreadCreated) {
      runNotifierThread = true;
      notifierThread = std::thread([]{
        NiFpga_IrqContext context;
        NiFpga_ReserveIrqContext(FPGASession, &context);
        while (runNotifierThread) {
          NiFpga_WaitOnIrqs(FPGASession, context, NiFpga_Irq_28, NiFpga_InfiniteTimeout, nullptr, nullptr);
          if (!runNotifierThread) break;
          alarmCallback();
        }
        NiFpga_UnreserveIrqContext(FPGASession, context);
      });
    }
  }

  std::shared_ptr<Notifier> notifier = std::make_shared<Notifier>();
  HAL_NotifierHandle handle = notifierHandles->Allocate(notifier);
  if (handle == HAL_kInvalidHandle) {
    *status = HAL_HANDLE_ERROR;
    return HAL_kInvalidHandle;
  }
  return handle;
}

void HAL_SetNotifierName(HAL_NotifierHandle notifierHandle, const char* name,
                         int32_t* status) {}

void HAL_StopNotifier(HAL_NotifierHandle notifierHandle, int32_t* status) {
  auto notifier = notifierHandles->Get(notifierHandle);
  if (!notifier) return;

  {
    std::scoped_lock lock(notifier->mutex);
    notifier->triggerTime = UINT64_MAX;
    notifier->triggeredTime = 0;
    notifier->active = false;
  }
  notifier->cond.notify_all();  // wake up any waiting threads
}

void HAL_CleanNotifier(HAL_NotifierHandle notifierHandle, int32_t* status) {
  auto notifier = notifierHandles->Free(notifierHandle);
  if (!notifier) return;

  // Just in case HAL_StopNotifier() wasn't called...
  {
    std::scoped_lock lock(notifier->mutex);
    notifier->triggerTime = UINT64_MAX;
    notifier->triggeredTime = 0;
    notifier->active = false;
  }
  notifier->cond.notify_all();

  if (notifierRefCount.fetch_sub(1) == 1) {
    // if this was the last notifier, clean up alarm and manager
    // the notifier can call back into our callback, so don't hold the lock
    // here (the atomic fetch_sub will prevent multiple parallel entries
    // into this function)

    // Cleaning up the manager takes up to a second to complete, so don't do
    // that here. Fix it more permanently in 2019...

    // if (notifierAlarm) notifierAlarm->writeEnable(false, status);
    // if (notifierManager) notifierManager->disable(status);

    // std::scoped_lock lock(notifierMutex);
    // notifierAlarm = nullptr;
    // notifierManager = nullptr;
    // closestTrigger = UINT64_MAX;
  }
}

void HAL_UpdateNotifierAlarm(HAL_NotifierHandle notifierHandle,
                             uint64_t triggerTime, int32_t* status) {
  auto notifier = notifierHandles->Get(notifierHandle);
  if (!notifier) return;

  {
    std::scoped_lock lock(notifier->mutex);
    notifier->triggerTime = triggerTime;
    notifier->triggeredTime = UINT64_MAX;
  }

  std::scoped_lock lock(notifierMutex);
  // Update alarm time if closer than current.
  if (triggerTime < closestTrigger) {
    bool wasActive = (closestTrigger != UINT64_MAX);
    closestTrigger = triggerTime;
    // Simply truncate the hardware trigger time to 32-bit.
    *status = NiFpga_WriteU32(FPGASession, NiFpga_OpenSourceRIO_ControlU32_Alarm_TriggerTime, static_cast<uint32_t>(closestTrigger));
    // Enable the alarm.
    if (!wasActive) {
      *status = NiFpga_WriteBool(FPGASession, NiFpga_OpenSourceRIO_ControlBool_Alarm_Enable, true);
    }
  }
}

void HAL_CancelNotifierAlarm(HAL_NotifierHandle notifierHandle,
                             int32_t* status) {
  auto notifier = notifierHandles->Get(notifierHandle);
  if (!notifier) return;

  {
    std::scoped_lock lock(notifier->mutex);
    notifier->triggerTime = UINT64_MAX;
  }
}

uint64_t HAL_WaitForNotifierAlarm(HAL_NotifierHandle notifierHandle,
                                  int32_t* status) {
  auto notifier = notifierHandles->Get(notifierHandle);
  if (!notifier) return 0;
  std::unique_lock lock(notifier->mutex);
  notifier->cond.wait(lock, [&] {
    return !notifier->active || notifier->triggeredTime != UINT64_MAX;
  });
  return notifier->active ? notifier->triggeredTime : 0;
}

}  // extern "C"
