#include "hal/DIO.h"

#include "hal/Errors.h"

#include <pthread.h>

#include "cHandlesInternal.h"


#include "FPGA.h"

static HAL_DigitalHandle dioHandles[26] = {0};

static pthread_mutex_t dioHandleMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t dioSettingMutex = PTHREAD_MUTEX_INITIALIZER;

HAL_DigitalHandle HAL_InitializeDIOPort(HAL_PortHandle portHandle, HAL_Bool input, int32_t* status) {
  int16_t channel = getPortHandleChannel(portHandle);
  if (channel < 0) {
    *status = HAL_HANDLE_ERROR;
    return HAL_kInvalidHandle;
  }

  pthread_mutex_lock(&dioHandleMutex);

  if (dioHandles[channel] != 0) {
    pthread_mutex_unlock(&dioHandleMutex);
    *status = RESOURCE_IS_ALLOCATED;
    return HAL_kInvalidHandle;
  }

  dioHandles[channel] = HAL_Handle_CreateHandle(channel, HAL_HandleEnum_DIO, 0);

  HAL_DigitalHandle handle = dioHandles[channel];

  pthread_mutex_unlock(&dioHandleMutex);

  pthread_mutex_lock(&dioSettingMutex);

  // TODO Set Direction

  pthread_mutex_unlock(&dioSettingMutex);

  return handle;
}

HAL_Bool HAL_GetDIO(HAL_DigitalHandle handle, int32_t* status) {
  int16_t channel = getHandleTypedIndex(handle, HAL_HandleEnum_DIO, 0);
  if (channel < 0) {
    *status = HAL_kInvalidHandle;
    return 0;
  }

  uint32_t value = 0;
  *status = NiFpga_ReadU32(HAL_FPGA_Session, NiFpga_OpenSourceRIO_IndicatorU32_Digital_Inputs, &value);
  return (value >> channel) & 1;
}
