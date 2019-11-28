/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2019 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "AnalogInternal.h"

#include <atomic>

#include <wpi/mutex.h>

#include "HALInitializer.h"
#include "PortsInternal.h"
#include "hal/AnalogInput.h"

#include "NiFpga_OpenSourceRIO.h"
#include "Globals.h"

using namespace hal::internal;

namespace hal {

wpi::mutex analogRegisterWindowMutex;
//std::unique_ptr<tAI> analogInputSystem;
//std::unique_ptr<tAO> analogOutputSystem;

IndexedHandleResource<HAL_AnalogInputHandle, ::hal::AnalogPort,
                      kNumAnalogInputs, HAL_HandleEnum::AnalogInput>*
    analogInputHandles;

static std::atomic<bool> analogSystemInitialized{false};

bool analogSampleRateSet = false;

namespace init {
void InitializeAnalogInternal() {
  static IndexedHandleResource<HAL_AnalogInputHandle, ::hal::AnalogPort,
                               kNumAnalogInputs, HAL_HandleEnum::AnalogInput>
      alH;
  analogInputHandles = &alH;
}
}  // namespace init

void initializeAnalog(int32_t* status) {
  hal::init::CheckInit();
  if (analogSystemInitialized) return;
  std::scoped_lock lock(analogRegisterWindowMutex);
  if (analogSystemInitialized) return;
  //analogInputSystem.reset(tAI::create(status));
  //analogOutputSystem.reset(tAO::create(status));
  setAnalogSampleRate(kDefaultSampleRate, status);
  analogSystemInitialized = true;
}

void setAnalogSampleRate(double samplesPerSecond, int32_t* status) {
  // TODO: This will change when variable size scan lists are implemented.
  // TODO: Need double comparison with epsilon.
  // wpi_assert(!sampleRateSet || GetSampleRate() == samplesPerSecond);
  analogSampleRateSet = true;

  // Compute the convert rate
  uint32_t ticksPerSample =
      static_cast<uint32_t>(static_cast<double>(kTimebase) / samplesPerSecond);

  NiFpga_WriteU32(FPGASession, NiFpga_OpenSourceRIO_ControlU32_Analog_SampleRate, ticksPerSample);
}

}  // namespace hal
