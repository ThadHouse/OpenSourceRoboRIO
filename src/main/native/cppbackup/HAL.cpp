#include "hal/HALBase.h"
#include "NiFpga_OpenSourceRIO.h"

#include "Globals.h"

#include <stdio.h>
#include <stdlib.h>

#include "hal/handles/HandlesInternal.h"

namespace hal {
namespace internal {
  NiFpga_Session FPGASession;
}
}

using namespace hal::internal;

static void ExitFunc() {
  if (FPGASession) {
    NiFpga_Close(FPGASession, 0);
  }
}

HAL_Bool HAL_Initialize(int32_t timeout, int32_t mode)
{
  NiFpga_Status status = NiFpga_Initialize();
  status = NiFpga_Open("/home/admin/" NiFpga_OpenSourceRIO_Bitfile,
                                     NiFpga_OpenSourceRIO_Signature, "RIO0", 0, &FPGASession);

  if (NiFpga_IsNotError(status)) {
    printf("Writing Enabled\n");
    NiFpga_WriteBool(FPGASession, NiFpga_OpenSourceRIO_ControlBool_PWM_Enabled, 1);
    NiFpga_WriteU16(FPGASession, NiFpga_OpenSourceRIO_ControlU16_PWM_MinHigh, 500);
    NiFpga_WriteU16(FPGASession, NiFpga_OpenSourceRIO_ControlU16_PWM_Period, 5000);
    std::atexit(ExitFunc);
  }

  return NiFpga_IsNotError(status);
}

uint64_t HAL_GetFPGATime(int32_t* status) {
  *status = 0;
  uint32_t upper1 = 0;
  uint32_t lower = 0;
  uint32_t upper2 = 0;
  *status = NiFpga_ReadU32(FPGASession, NiFpga_OpenSourceRIO_IndicatorU32_Timing_Upper, &upper1);
  *status = NiFpga_ReadU32(FPGASession, NiFpga_OpenSourceRIO_IndicatorU32_Timing_Lower, &lower);
  *status = NiFpga_ReadU32(FPGASession, NiFpga_OpenSourceRIO_IndicatorU32_Timing_Upper, &upper2);
  if (NiFpga_IsError(*status)) {
    return 0;
  }

  if (upper1 != upper2) {
    *status = NiFpga_ReadU32(FPGASession, NiFpga_OpenSourceRIO_IndicatorU32_Timing_Lower, &lower);
    if (NiFpga_IsError(*status)) {
      return 0;
    }
  }
  return (((uint64_t)upper2) << 32) + lower;

}

HAL_Bool HAL_GetFPGAButton(int32_t* status) {
  NiFpga_Bool retVal = 0;
  *status = NiFpga_ReadBool(FPGASession, NiFpga_OpenSourceRIO_IndicatorBool_UserButton, &retVal);
  return retVal;
}

HAL_PortHandle HAL_GetPort(int32_t channel) {
  // Dont allow a number that wouldn't fit in a uint8_t
  if (channel < 0 || channel >= 255) return HAL_kInvalidHandle;
  return hal::createPortHandle(channel, 1);
}
