#include "hal/PWM.h"

#include "hal/Errors.h"

#include <pthread.h>

#include "cHandlesInternal.h"
#include "NiFpga_OpenSourceRIO.h"

#include "Globals.h"


#include "stdio.h"

static HAL_DigitalHandle pwmHandles[10] = {0};

static pthread_mutex_t pwmHandleMutex = PTHREAD_MUTEX_INITIALIZER;

HAL_DigitalHandle HAL_InitializePWMPort(HAL_PortHandle portHandle, int32_t* status) {
  int16_t channel = getPortHandleChannel(portHandle);
  if (channel < 0) {
    *status = HAL_HANDLE_ERROR;
    return HAL_kInvalidHandle;
  }

  pthread_mutex_lock(&pwmHandleMutex);

  if (pwmHandles[channel] != 0) {
    pthread_mutex_unlock(&pwmHandleMutex);
    *status = RESOURCE_IS_ALLOCATED;
    return HAL_kInvalidHandle;
  }

  pwmHandles[channel] = HAL_Handle_CreateHandle(channel, HAL_HandleEnum_PWM, 0);

  HAL_DigitalHandle handle = pwmHandles[channel];

  pthread_mutex_unlock(&pwmHandleMutex);

  *status = NiFpga_WriteU32(HAL_FPGA_Session, NiFpga_OpenSourceRIO_ControlU32_PWM_SetValue, ((uint32_t)channel << 16) | 1500);

  if (*status != 0) {
    return HAL_kInvalidHandle;
  }

  return handle;
}

void HAL_SetPWMRaw(HAL_DigitalHandle handle, int32_t value,
                   int32_t* status) {
  int16_t channel = getHandleTypedIndex(handle, HAL_HandleEnum_PWM, 0);
  if (channel < 0) {
    *status = HAL_kInvalidHandle;
    return;
  }

  //printf("Writing: %d",  ((uint32_t)channel << 16) | (value & 0xFFFF));

  *status = NiFpga_WriteU32(HAL_FPGA_Session, NiFpga_OpenSourceRIO_ControlU32_PWM_SetValue, ((uint32_t)channel << 16) | (value & 0xFFFF));
}
