#include "hal/AnalogOutput.h"

#include "hal/Errors.h"
#include "AnalogInternal.h"
#include "hal/handles/HandlesInternal.h"
#include "HALInitializer.h"
#include "hal/handles/IndexedHandleResource.h"
#include "PortsInternal.h"
#include "NiFpga_OpenSourceRIO.h"
#include "Globals.h"

using namespace hal::internal;

using namespace hal;

namespace {

struct AnalogOutput {
  uint8_t channel;
  uint32_t control;
};

}  // namespace

static IndexedHandleResource<HAL_AnalogOutputHandle, AnalogOutput,
                             kNumAnalogOutputs, HAL_HandleEnum::AnalogOutput>*
    analogOutputHandles;

namespace hal {
namespace init {
void InitializeAnalogOutput() {
  static IndexedHandleResource<HAL_AnalogOutputHandle, AnalogOutput,
                               kNumAnalogOutputs, HAL_HandleEnum::AnalogOutput>
      aoH;
  analogOutputHandles = &aoH;
}
}  // namespace init
}  // namespace hal

extern "C" {

HAL_AnalogOutputHandle HAL_InitializeAnalogOutputPort(HAL_PortHandle portHandle,
                                                      int32_t* status) {
  hal::init::CheckInit();
  initializeAnalog(status);

  if (*status != 0) return HAL_kInvalidHandle;

  int16_t channel = getPortHandleChannel(portHandle);
  if (channel == InvalidHandleIndex) {
    *status = PARAMETER_OUT_OF_RANGE;
    return HAL_kInvalidHandle;
  }

  HAL_AnalogOutputHandle handle =
      analogOutputHandles->Allocate(channel, status);

  if (*status != 0)
    return HAL_kInvalidHandle;  // failed to allocate. Pass error back.

  auto port = analogOutputHandles->Get(handle);
  if (port == nullptr) {  // would only error on thread issue
    *status = HAL_HANDLE_ERROR;
    return HAL_kInvalidHandle;
  }

  port->channel = static_cast<uint8_t>(channel);
  if (channel == 0) {
    port->control = NiFpga_OpenSourceRIO_ControlU16_Analog_AO0;
  } else {
    port->control = NiFpga_OpenSourceRIO_ControlU16_Analog_AO1;
  }
  return handle;
}

void HAL_FreeAnalogOutputPort(HAL_AnalogOutputHandle analogOutputHandle) {
  // no status, so no need to check for a proper free.
  analogOutputHandles->Free(analogOutputHandle);
}

void HAL_SetAnalogOutput(HAL_AnalogOutputHandle analogOutputHandle,
                         double voltage, int32_t* status) {
  auto port = analogOutputHandles->Get(analogOutputHandle);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  uint16_t rawValue = static_cast<uint16_t>(voltage / 5.0 * 0x1000);

  if (voltage < 0.0)
    rawValue = 0;
  else if (voltage > 5.0)
    rawValue = 0x1000;

  *status = NiFpga_WriteU16(FPGASession, port->control, rawValue);
}

double HAL_GetAnalogOutput(HAL_AnalogOutputHandle analogOutputHandle,
                           int32_t* status) {
  auto port = analogOutputHandles->Get(analogOutputHandle);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0.0;
  }

  uint16_t rawValue = 0;

  *status = NiFpga_ReadU16(FPGASession, port->control, &rawValue);

  return rawValue * 5.0 / 0x1000;
}

HAL_Bool HAL_CheckAnalogOutputChannel(int32_t channel) {
  return channel < kNumAnalogOutputs && channel >= 0;
}

}
