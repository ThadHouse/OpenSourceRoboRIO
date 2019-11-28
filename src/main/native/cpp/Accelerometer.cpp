#include "hal/Accelerometer.h"

#include "NiFpga_OpenSourceRIO.h"
#include "Globals.h"

using namespace hal::internal;

using namespace hal;

namespace hal {
namespace init {
void InitializeAccelerometer() {}
}  // namespace init
}  // namespace hal

extern "C" {

void HAL_SetAccelerometerActive(HAL_Bool active) {
  // No op
}

void HAL_SetAccelerometerRange(HAL_AccelerometerRange range) {
  // No op, not supported
}

double HAL_GetAccelerometerX(void) {
  uint16_t value = 0;
  auto status = NiFpga_ReadU16(FPGASession, NiFpga_OpenSourceRIO_IndicatorI16_AccelerationX, &value);
  if (NiFpga_IsError(status)) return 0;
  return static_cast<double>(value) * ( 16.0 / 4096);
}

double HAL_GetAccelerometerY(void) {
  uint16_t value = 0;
  auto status = NiFpga_ReadU16(FPGASession, NiFpga_OpenSourceRIO_IndicatorI16_AccelerationY, &value);
  if (NiFpga_IsError(status)) return 0;
  return static_cast<double>(value) * ( 16.0 / 4096);
}

double HAL_GetAccelerometerZ(void) {
  uint16_t value = 0;
  auto status = NiFpga_ReadU16(FPGASession, NiFpga_OpenSourceRIO_IndicatorI16_AccelerationZ, &value);
  if (NiFpga_IsError(status)) return 0;
  return static_cast<double>(value) * ( 16.0 / 4096);
}

}
