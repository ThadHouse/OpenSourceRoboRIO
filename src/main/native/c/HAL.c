#include "hal/HALBase.h"
#include "NiFpga_OpenSourceRIO.h"

#include "cHandlesInternal.h"

#include <stdio.h>
#include <stdlib.h>

NiFpga_Session HAL_FPGA_Session = 0;

static void ExitFunc() {
  if (HAL_FPGA_Session) {
    NiFpga_Close(HAL_FPGA_Session, 0);
  }
}

HAL_Bool HAL_Initialize(int32_t timeout, int32_t mode)
{
  NiFpga_Status status = NiFpga_Initialize();
  status = NiFpga_Open("/home/admin/" NiFpga_OpenSourceRIO_Bitfile,
                                     NiFpga_OpenSourceRIO_Signature, "RIO0", 0, &HAL_FPGA_Session);

  if (NiFpga_IsNotError(status)) {
    printf("Writing Enabled\n");
    NiFpga_WriteBool(HAL_FPGA_Session, NiFpga_OpenSourceRIO_ControlBool_PWM_Enabled, 1);
    NiFpga_WriteU16(HAL_FPGA_Session, NiFpga_OpenSourceRIO_ControlU16_PWM_MinHigh, 500);
    NiFpga_WriteU16(HAL_FPGA_Session, NiFpga_OpenSourceRIO_ControlU16_PWM_Period, 5000);
    atexit(ExitFunc);
  }

  printf("%d", status);



  return NiFpga_IsNotError(status);
}

uint64_t HAL_GetFPGATime(int32_t* status) {
  *status = 0;
  uint32_t upper1 = 0;
  uint32_t lower = 0;
  uint32_t upper2 = 0;
  *status = NiFpga_ReadU32(HAL_FPGA_Session, NiFpga_OpenSourceRIO_IndicatorU32_Timing_Upper, &upper1);
  *status = NiFpga_ReadU32(HAL_FPGA_Session, NiFpga_OpenSourceRIO_IndicatorU32_Timing_Lower, &lower);
  *status = NiFpga_ReadU32(HAL_FPGA_Session, NiFpga_OpenSourceRIO_IndicatorU32_Timing_Upper, &upper2);
  if (NiFpga_IsError(*status)) {
    return 0;
  }

  if (upper1 != upper2) {
    *status = NiFpga_ReadU32(HAL_FPGA_Session, NiFpga_OpenSourceRIO_IndicatorU32_Timing_Lower, &lower);
    if (NiFpga_IsError(*status)) {
      return 0;
    }
  }
  return (((uint64_t)upper2) << 32) + lower;

}

HAL_Bool HAL_GetFPGAButton(int32_t* status) {
  NiFpga_Bool retVal = 0;
  *status = NiFpga_ReadBool(HAL_FPGA_Session, NiFpga_OpenSourceRIO_IndicatorBool_UserButton, &retVal);
  return retVal;
}

HAL_PortHandle HAL_GetPort(int32_t channel) {
  // Dont allow a number that wouldn't fit in a uint8_t
  if (channel < 0 || channel >= 255) return HAL_kInvalidHandle;
  return HAL_Handle_CreatePortHandle(channel, 1);
}



HAL_PortHandle HAL_Handle_CreatePortHandle(uint8_t channel, uint8_t module) {
  // set last 8 bits, then shift to first 8 bits
  HAL_PortHandle handle = HAL_HandleEnum_Port;
  handle = handle << 24;
  // shift module and add to 3rd set of 8 bits
  int32_t temp = module;
  temp = (temp << 8) & 0xff00;
  handle += temp;
  // add channel to last 8 bits
  handle += channel;
  return handle;
}

HAL_Handle HAL_Handle_CreateHandle(int16_t index, enum HAL_HandleEnum handleType,
                        int16_t version) {
  if (index < 0) return HAL_kInvalidHandle;
  uint8_t hType = (handleType);
  if (hType == 0 || hType > 127) return HAL_kInvalidHandle;
  // set last 8 bits, then shift to first 8 bits
  HAL_Handle handle = hType;
  handle = handle << 8;
  handle += (uint8_t)(version);
  handle = handle << 16;
  // add index to set last 16 bits
  handle += index;
  return handle;
}
