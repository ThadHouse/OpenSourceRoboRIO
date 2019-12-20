/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "hal/Counter.h"

#include "ConstantsInternal.h"
#include "FPGA.h"
#include "DigitalInternal.h"
#include "HALInitializer.h"
#include "PortsInternal.h"
#include "hal/HAL.h"
#include "hal/handles/LimitedHandleResource.h"

using namespace hal;

namespace {

struct Counter {
  uint32_t configControl;
  uint32_t timerConfigControl;
  uint32_t outputIndicator;
  uint32_t timerOutputIndicator;
  uint32_t resetControl;
  uint8_t index;
};

}  // namespace

static LimitedHandleResource<HAL_CounterHandle, Counter, kNumCounters,
                             HAL_HandleEnum::Counter>* counterHandles;

namespace hal {
namespace init {
void InitializeCounter() {
  static LimitedHandleResource<HAL_CounterHandle, Counter, kNumCounters,
                               HAL_HandleEnum::Counter>
      ch;
  counterHandles = &ch;
}
}  // namespace init
}  // namespace hal

typedef NiFpga_OpenSourceRIO_ControlCluster_Counter0_Config_Type CounterConfig;
typedef NiFpga_OpenSourceRIO_ControlCluster_Counter0_TimerConfig_Type CounterTimerConfig;
typedef NiFpga_OpenSourceRIO_IndicatorCluster_Counter0_Output_Type CounterOutput;
typedef NiFpga_OpenSourceRIO_IndicatorCluster_Counter0_TimerOutput_Type CounterTimerOutput;

static int32_t ReadConfig(const Counter& counter, CounterConfig* config) {
  uint8_t data[NiFpga_OpenSourceRIO_ControlCluster_Counter0_Config_PackedSizeInBytes];
  int32_t status = NiFpga_ReadArrayU8(FPGASession, counter.configControl, data, sizeof(data));
  NiFpga_OpenSourceRIO_ControlCluster_Counter0_Config_UnpackCluster(data, config);
  return status;
}

static int32_t ReadTimerConfig(const Counter& counter, CounterTimerConfig* config) {
  uint8_t data[NiFpga_OpenSourceRIO_ControlCluster_Counter0_TimerConfig_PackedSizeInBytes];
  int32_t status = NiFpga_ReadArrayU8(FPGASession, counter.timerConfigControl, data, sizeof(data));
  NiFpga_OpenSourceRIO_ControlCluster_Counter0_TimerConfig_UnpackCluster(data, config);
  return status;
}

static int32_t WriteConfig(const Counter& counter, const CounterConfig& config) {
  uint8_t data[NiFpga_OpenSourceRIO_ControlCluster_Counter0_Config_PackedSizeInBytes];
  NiFpga_OpenSourceRIO_ControlCluster_Counter0_Config_PackCluster(data, &config);
  return NiFpga_WriteArrayU8(FPGASession, counter.configControl, data, sizeof(data));
}

static int32_t WriteTimerConfig(const Counter& counter, const CounterTimerConfig& config) {
  uint8_t data[NiFpga_OpenSourceRIO_ControlCluster_Counter0_TimerConfig_PackedSizeInBytes];
  NiFpga_OpenSourceRIO_ControlCluster_Counter0_TimerConfig_PackCluster(data, &config);
  return NiFpga_WriteArrayU8(FPGASession, counter.timerConfigControl, data, sizeof(data));
}

static int32_t ReadOutput(const Counter& counter, CounterOutput* config) {
  uint8_t data[NiFpga_OpenSourceRIO_IndicatorCluster_Counter0_Output_PackedSizeInBytes];
  int32_t status = NiFpga_ReadArrayU8(FPGASession, counter.outputIndicator, data, sizeof(data));
  NiFpga_OpenSourceRIO_IndicatorCluster_Counter0_Output_UnpackCluster(data, config);
  return status;
}

static int32_t ReadTimerOutput(const Counter& counter, CounterTimerOutput* config) {
  uint8_t data[NiFpga_OpenSourceRIO_IndicatorCluster_Counter0_TimerOutput_PackedSizeInBytes];
  int32_t status = NiFpga_ReadArrayU8(FPGASession, counter.timerOutputIndicator, data, sizeof(data));
  NiFpga_OpenSourceRIO_IndicatorCluster_Counter0_TimerOutput_UnpackCluster(data, config);
  return status;
}

extern "C" {

HAL_CounterHandle HAL_InitializeCounter(HAL_Counter_Mode mode, int32_t* index,
                                        int32_t* status) {
  hal::init::CheckInit();
  auto handle = counterHandles->Allocate();
  if (handle == HAL_kInvalidHandle) {  // out of resources
    *status = NO_AVAILABLE_RESOURCES;
    return HAL_kInvalidHandle;
  }
  auto counter = counterHandles->Get(handle);
  if (counter == nullptr) {  // would only occur on thread issues
    *status = HAL_HANDLE_ERROR;
    return HAL_kInvalidHandle;
  }
  counter->index = static_cast<uint8_t>(getHandleIndex(handle));
  *index = counter->index;

  switch (*index) {
    case 1:
      counter->configControl = NiFpga_OpenSourceRIO_ControlCluster_Counter1_Config_Resource;
      counter->timerConfigControl = NiFpga_OpenSourceRIO_ControlCluster_Counter1_TimerConfig_Resource;
      counter->resetControl = NiFpga_OpenSourceRIO_ControlBool_Counter1_Reset;
      counter->outputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter1_Output_Resource;
      counter->timerOutputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter1_TimerOutput_Resource;
      break;
          case 2:
      counter->configControl = NiFpga_OpenSourceRIO_ControlCluster_Counter2_Config_Resource;
      counter->timerConfigControl = NiFpga_OpenSourceRIO_ControlCluster_Counter2_TimerConfig_Resource;
      counter->resetControl = NiFpga_OpenSourceRIO_ControlBool_Counter2_Reset;
      counter->outputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter2_Output_Resource;
      counter->timerOutputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter2_TimerOutput_Resource;
      break;
          case 3:
      counter->configControl = NiFpga_OpenSourceRIO_ControlCluster_Counter3_Config_Resource;
      counter->timerConfigControl = NiFpga_OpenSourceRIO_ControlCluster_Counter3_TimerConfig_Resource;
      counter->resetControl = NiFpga_OpenSourceRIO_ControlBool_Counter3_Reset;
      counter->outputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter3_Output_Resource;
      counter->timerOutputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter3_TimerOutput_Resource;
      break;
          case 4:
      counter->configControl = NiFpga_OpenSourceRIO_ControlCluster_Counter4_Config_Resource;
      counter->timerConfigControl = NiFpga_OpenSourceRIO_ControlCluster_Counter4_TimerConfig_Resource;
      counter->resetControl = NiFpga_OpenSourceRIO_ControlBool_Counter4_Reset;
      counter->outputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter4_Output_Resource;
      counter->timerOutputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter4_TimerOutput_Resource;
      break;
          case 5:
      counter->configControl = NiFpga_OpenSourceRIO_ControlCluster_Counter5_Config_Resource;
      counter->timerConfigControl = NiFpga_OpenSourceRIO_ControlCluster_Counter5_TimerConfig_Resource;
      counter->resetControl = NiFpga_OpenSourceRIO_ControlBool_Counter5_Reset;
      counter->outputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter5_Output_Resource;
      counter->timerOutputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter5_TimerOutput_Resource;
      break;
          case 6:
      counter->configControl = NiFpga_OpenSourceRIO_ControlCluster_Counter6_Config_Resource;
      counter->timerConfigControl = NiFpga_OpenSourceRIO_ControlCluster_Counter6_TimerConfig_Resource;
      counter->resetControl = NiFpga_OpenSourceRIO_ControlBool_Counter6_Reset;
      counter->outputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter6_Output_Resource;
      counter->timerOutputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter6_TimerOutput_Resource;
      break;
          case 7:
      counter->configControl = NiFpga_OpenSourceRIO_ControlCluster_Counter7_Config_Resource;
      counter->timerConfigControl = NiFpga_OpenSourceRIO_ControlCluster_Counter7_TimerConfig_Resource;
      counter->resetControl = NiFpga_OpenSourceRIO_ControlBool_Counter7_Reset;
      counter->outputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter7_Output_Resource;
      counter->timerOutputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter7_TimerOutput_Resource;
      break;
    default:
      counter->configControl = NiFpga_OpenSourceRIO_ControlCluster_Counter0_Config_Resource;
      counter->timerConfigControl = NiFpga_OpenSourceRIO_ControlCluster_Counter0_TimerConfig_Resource;
      counter->resetControl = NiFpga_OpenSourceRIO_ControlBool_Counter0_Reset;
      counter->outputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter0_Output_Resource;
      counter->timerOutputIndicator = NiFpga_OpenSourceRIO_IndicatorCluster_Counter0_TimerOutput_Resource;
      break;
  }

  CounterConfig config;
  *status = ReadConfig(*counter, &config);
  config.Mode = mode;
  *status = WriteConfig(*counter, config);

  CounterTimerConfig timerConfig;
  *status = ReadTimerConfig(*counter, &timerConfig);
  timerConfig.AverageSize = 1;
  *status = WriteTimerConfig(*counter, timerConfig);

  return handle;
}

void HAL_FreeCounter(HAL_CounterHandle counterHandle, int32_t* status) {
  counterHandles->Free(counterHandle);
}

void HAL_SetCounterAverageSize(HAL_CounterHandle counterHandle, int32_t size,
                               int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterTimerConfig timerConfig;
  *status = ReadTimerConfig(*counter, &timerConfig);
  timerConfig.AverageSize = size;
  *status = WriteTimerConfig(*counter, timerConfig);
}

void HAL_SetCounterUpSource(HAL_CounterHandle counterHandle,
                            HAL_Handle digitalSourceHandle,
                            HAL_AnalogTriggerType analogTriggerType,
                            int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  bool routingAnalogTrigger = false;
  uint8_t routingChannel = 0;
  uint8_t routingModule = 0;
  bool success =
      remapDigitalSource(digitalSourceHandle, analogTriggerType, routingChannel,
                         routingModule, routingAnalogTrigger);
  if (!success) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterConfig config;
  *status = ReadConfig(*counter, &config);

  config.ASource.Module = routingModule;
  config.ASource.Channel = routingChannel;
  config.ASource.AnalogTrigger = routingAnalogTrigger;

  if (config.Mode == HAL_Counter_kTwoPulse || config.Mode == HAL_Counter_kExternalDirection) {
    config.UpRisingEdge = true;
    config.UpFallingEdge = false;
  }

  *status = WriteConfig(*counter, config);

  NiFpga_WriteBool(FPGASession, counter->resetControl, true);
}

void HAL_SetCounterUpSourceEdge(HAL_CounterHandle counterHandle,
                                HAL_Bool risingEdge, HAL_Bool fallingEdge,
                                int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterConfig config;
  *status = ReadConfig(*counter, &config);

  config.UpRisingEdge = risingEdge;
  config.UpFallingEdge = fallingEdge;

  *status = WriteConfig(*counter, config);
}

void HAL_ClearCounterUpSource(HAL_CounterHandle counterHandle,
                              int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterConfig config;
  *status = ReadConfig(*counter, &config);

  config.UpRisingEdge = false;
  config.UpFallingEdge = false;
  config.ASource.Module = 0;
  config.ASource.Channel = 0;
  config.ASource.AnalogTrigger = 0;

  *status = WriteConfig(*counter, config);
}

void HAL_SetCounterDownSource(HAL_CounterHandle counterHandle,
                              HAL_Handle digitalSourceHandle,
                              HAL_AnalogTriggerType analogTriggerType,
                              int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterConfig config;
  *status = ReadConfig(*counter, &config);

  uint8_t mode = config.Mode;
  if (mode != HAL_Counter_kTwoPulse && mode != HAL_Counter_kExternalDirection) {
    // TODO: wpi_setWPIErrorWithContext(ParameterOutOfRange, "Counter only
    // supports DownSource in TwoPulse and ExternalDirection modes.");
    *status = PARAMETER_OUT_OF_RANGE;
    return;
  }

  bool routingAnalogTrigger = false;
  uint8_t routingChannel = 0;
  uint8_t routingModule = 0;
  bool success =
      remapDigitalSource(digitalSourceHandle, analogTriggerType, routingChannel,
                         routingModule, routingAnalogTrigger);
  if (!success) {
    *status = HAL_HANDLE_ERROR;
    return;
  }


  config.BSource.Module = routingModule;
  config.BSource.Channel = routingChannel;
  config.BSource.AnalogTrigger = routingAnalogTrigger;

  config.DownRisingEdge = true;
  config.DownFallingEdge = false;

  *status = WriteConfig(*counter, config);

  NiFpga_WriteBool(FPGASession, counter->resetControl, true);
}

void HAL_SetCounterDownSourceEdge(HAL_CounterHandle counterHandle,
                                  HAL_Bool risingEdge, HAL_Bool fallingEdge,
                                  int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterConfig config;
  *status = ReadConfig(*counter, &config);

  config.DownRisingEdge = risingEdge;
  config.DownFallingEdge = fallingEdge;

  *status = WriteConfig(*counter, config);
}

void HAL_ClearCounterDownSource(HAL_CounterHandle counterHandle,
                                int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterConfig config;
  *status = ReadConfig(*counter, &config);

  config.DownRisingEdge = false;
  config.DownFallingEdge = false;
  config.BSource.Module = 0;
  config.BSource.Channel = 0;
  config.BSource.AnalogTrigger = 0;

  *status = WriteConfig(*counter, config);
}

void HAL_SetCounterUpDownMode(HAL_CounterHandle counterHandle,
                              int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterConfig config;
  *status = ReadConfig(*counter, &config);

  config.Mode = HAL_Counter_kTwoPulse;

  *status = WriteConfig(*counter, config);
}

void HAL_SetCounterExternalDirectionMode(HAL_CounterHandle counterHandle,
                                         int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterConfig config;
  *status = ReadConfig(*counter, &config);

  config.Mode = HAL_Counter_kExternalDirection;

  *status = WriteConfig(*counter, config);
}

void HAL_SetCounterSemiPeriodMode(HAL_CounterHandle counterHandle,
                                  HAL_Bool highSemiPeriod, int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterConfig config;
  *status = ReadConfig(*counter, &config);

  config.Mode = HAL_Counter_kSemiperiod;
  config.UpRisingEdge = highSemiPeriod;

  *status = WriteConfig(*counter, config);

  CounterTimerConfig timerConfig;
  *status = ReadTimerConfig(*counter, &timerConfig);

  timerConfig.UpdateWhenEmpty = false;

  *status = WriteTimerConfig(*counter, timerConfig);
}

void HAL_SetCounterPulseLengthMode(HAL_CounterHandle counterHandle,
                                   double threshold, int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterConfig config;
  *status = ReadConfig(*counter, &config);

  config.Mode = HAL_Counter_kPulseLength;
  config.PulseLengthThreshold = static_cast<uint32_t>(threshold * 1.0e6);

  *status = WriteConfig(*counter, config);
}

int32_t HAL_GetCounterSamplesToAverage(HAL_CounterHandle counterHandle,
                                       int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }

  CounterTimerConfig timerConfig;
  *status = ReadTimerConfig(*counter, &timerConfig);
  return timerConfig.AverageSize;
}

void HAL_SetCounterSamplesToAverage(HAL_CounterHandle counterHandle,
                                    int32_t samplesToAverage, int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  if (samplesToAverage < 1 || samplesToAverage > 127) {
    *status = PARAMETER_OUT_OF_RANGE;
  }

  CounterTimerConfig timerConfig;
  *status = ReadTimerConfig(*counter, &timerConfig);
  timerConfig.AverageSize = samplesToAverage;
  *status = WriteTimerConfig(*counter, timerConfig);
}

void HAL_ResetCounter(HAL_CounterHandle counterHandle, int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  NiFpga_WriteBool(FPGASession, counter->resetControl, true);
}

int32_t HAL_GetCounter(HAL_CounterHandle counterHandle, int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }

  CounterOutput output;
  *status = ReadOutput(*counter, &output);
  return output.Value;
}

double HAL_GetCounterPeriod(HAL_CounterHandle counterHandle, int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0.0;
  }

  CounterTimerOutput output;
  *status = ReadTimerOutput(*counter, &output);

  return 0.0; // TODO: Fix Period
}

void HAL_SetCounterMaxPeriod(HAL_CounterHandle counterHandle, double maxPeriod,
                             int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterTimerConfig timerConfig;
  *status = ReadTimerConfig(*counter, &timerConfig);
  timerConfig.StallPeriod = static_cast<uint32_t>(maxPeriod * 4.0e8);
  *status = WriteTimerConfig(*counter, timerConfig);
}

void HAL_SetCounterUpdateWhenEmpty(HAL_CounterHandle counterHandle,
                                   HAL_Bool enabled, int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  CounterTimerConfig timerConfig;
  *status = ReadTimerConfig(*counter, &timerConfig);
  timerConfig.UpdateWhenEmpty = enabled;
  *status = WriteTimerConfig(*counter, timerConfig);
}

HAL_Bool HAL_GetCounterStopped(HAL_CounterHandle counterHandle,
                               int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }

  CounterTimerOutput output;
  *status = ReadTimerOutput(*counter, &output);
  return output.Stopped;
}

HAL_Bool HAL_GetCounterDirection(HAL_CounterHandle counterHandle,
                                 int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }

  CounterOutput output;
  *status = ReadOutput(*counter, &output);
  return output.Direction;
}

void HAL_SetCounterReverseDirection(HAL_CounterHandle counterHandle,
                                    HAL_Bool reverseDirection,
                                    int32_t* status) {
  auto counter = counterHandles->Get(counterHandle);
  if (counter == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  // if (counter->counter->readConfig_Mode(status)
  //     HAL_Counter_kExternalDirection) {
  //   if (reverseDirection)
  //     HAL_SetCounterDownSourceEdge(counterHandle, true, true, status);
  //   else
  //     HAL_SetCounterDownSourceEdge(counterHandle, false, true, status);
  // }
}

}  // extern "C"
