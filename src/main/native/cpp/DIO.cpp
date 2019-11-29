#include "hal/DIO.h"

#include <cmath>
#include <thread>

#include <wpi/raw_ostream.h>

#include "DigitalInternal.h"
#include "HALInitializer.h"
#include "PortsInternal.h"
#include "hal/cpp/fpga_clock.h"
#include "hal/handles/HandlesInternal.h"
#include "hal/handles/LimitedHandleResource.h"

#include "NiFpga_OpenSourceRIO.h"
#include "Globals.h"

using namespace hal::internal;

using namespace hal;

// Create a mutex to protect changes to the DO PWM config
static wpi::mutex digitalPwmMutex;

static LimitedHandleResource<HAL_DigitalPWMHandle, uint8_t,
                             kNumDigitalPWMOutputs, HAL_HandleEnum::DigitalPWM>*
    digitalPWMHandles;

namespace hal {
namespace init {
void InitializeDIO() {
  static LimitedHandleResource<HAL_DigitalPWMHandle, uint8_t,
                               kNumDigitalPWMOutputs,
                               HAL_HandleEnum::DigitalPWM>
      dpH;
  digitalPWMHandles = &dpH;
}
}  // namespace init
}  // namespace hal

extern "C" {
HAL_DigitalHandle HAL_InitializeDIOPort(HAL_PortHandle portHandle,
                                        HAL_Bool input, int32_t* status) {
  hal::init::CheckInit();
  initializeDigital(status);

  if (*status != 0) return HAL_kInvalidHandle;

  int16_t channel = getPortHandleChannel(portHandle);
  if (channel == InvalidHandleIndex || channel >= kNumDigitalChannels) {
    *status = PARAMETER_OUT_OF_RANGE;
    return HAL_kInvalidHandle;
  }

  auto handle =
      digitalChannelHandles->Allocate(channel, HAL_HandleEnum::DIO, status);

  if (*status != 0)
    return HAL_kInvalidHandle;  // failed to allocate. Pass error back.

  auto port = digitalChannelHandles->Get(handle, HAL_HandleEnum::DIO);
  if (port == nullptr) {  // would only occur on thread issue.
    *status = HAL_HANDLE_ERROR;
    return HAL_kInvalidHandle;
  }

  port->channel = static_cast<uint8_t>(channel);

  std::scoped_lock lock(digitalDIOMutex);

  // TODO: Handle switching to inputs

  return handle;
}

void HAL_FreeDIOPort(HAL_DigitalHandle dioPortHandle) {
  auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
  // no status, so no need to check for a proper free.
  if (port == nullptr) return;
  digitalChannelHandles->Free(dioPortHandle, HAL_HandleEnum::DIO);

  // Wait for no other object to hold this handle.
  auto start = hal::fpga_clock::now();
  while (port.use_count() != 1) {
    auto current = hal::fpga_clock::now();
    if (start + std::chrono::seconds(1) < current) {
      wpi::outs() << "DIO handle free timeout\n";
      wpi::outs().flush();
      break;
    }
    std::this_thread::yield();
  }
}

HAL_Bool HAL_GetDIO(HAL_DigitalHandle dioPortHandle, int32_t* status) {
  auto port = digitalChannelHandles->Get(dioPortHandle, HAL_HandleEnum::DIO);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }

  auto idx = remapDigitalChannelToBitfieldChannel(port->channel);

  uint32_t value = 0;
  *status = NiFpga_ReadU32(FPGASession, NiFpga_OpenSourceRIO_IndicatorU32_Digital_Inputs, &value);
  return value >> idx & 0x1;
}
}
