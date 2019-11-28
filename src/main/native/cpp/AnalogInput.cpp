#include "hal/AnalogInput.h"

#include <wpi/mutex.h>

#include "AnalogInternal.h"
#include "HALInitializer.h"
#include "PortsInternal.h"
#include "hal/AnalogAccumulator.h"
#include "hal/handles/HandlesInternal.h"

#include "NiFpga_OpenSourceRIO.h"
#include "Globals.h"

using namespace hal::internal;

using namespace hal;

namespace hal {
namespace init {
void InitializeAnalogInput() {}
}  // namespace init
}  // namespace hal

extern "C"
{

  HAL_AnalogInputHandle HAL_InitializeAnalogInputPort(HAL_PortHandle portHandle,
                                                      int32_t *status)
  {
    hal::init::CheckInit();
    initializeAnalog(status);

    if (*status != 0)
      return HAL_kInvalidHandle;

    int16_t channel = getPortHandleChannel(portHandle);
    if (channel == InvalidHandleIndex)
    {
      *status = PARAMETER_OUT_OF_RANGE;
      return HAL_kInvalidHandle;
    }

    HAL_AnalogInputHandle handle = analogInputHandles->Allocate(channel, status);

    if (*status != 0)
      return HAL_kInvalidHandle; // failed to allocate. Pass error back.

    // Initialize port structure
    auto analog_port = analogInputHandles->Get(handle);
    if (analog_port == nullptr)
    { // would only error on thread issue
      *status = HAL_HANDLE_ERROR;
      return HAL_kInvalidHandle;
    }
    analog_port->channel = static_cast<uint8_t>(channel);
    // if (HAL_IsAccumulatorChannel(handle, status))
    // {
    //   //analog_port->accumulator.reset(tAccumulator::create(channel, status));
    // }
    // else
    // {
    //   //analog_port->accumulator = nullptr;
    // }

    switch (channel)
    {
    case 1:
      analog_port->control = NiFpga_OpenSourceRIO_IndicatorU16_Analog_AI1;
      break;
    case 2:
      analog_port->control = NiFpga_OpenSourceRIO_IndicatorU16_Analog_AI2;
      break;
    case 3:
      analog_port->control = NiFpga_OpenSourceRIO_IndicatorU16_Analog_AI3;
      break;
    case 4:
      analog_port->control = NiFpga_OpenSourceRIO_IndicatorU16_Analog_AI4;
      break;
    case 5:
      analog_port->control = NiFpga_OpenSourceRIO_IndicatorU16_Analog_AI5;
      break;
    case 6:
      analog_port->control = NiFpga_OpenSourceRIO_IndicatorU16_Analog_AI6;
      break;
    case 7:
      analog_port->control = NiFpga_OpenSourceRIO_IndicatorU16_Analog_AI7;
      break;
    default:
      analog_port->control = NiFpga_OpenSourceRIO_IndicatorU16_Analog_AI0;
      break;
    }

    // Set default configuration
    HAL_SetAnalogAverageBits(handle, kDefaultAverageBits, status);
    HAL_SetAnalogOversampleBits(handle, kDefaultOversampleBits, status);
    return handle;
  }

  void HAL_FreeAnalogInputPort(HAL_AnalogInputHandle analogPortHandle)
  {
    // no status, so no need to check for a proper free.
    analogInputHandles->Free(analogPortHandle);
  }

  HAL_Bool HAL_CheckAnalogModule(int32_t module) { return module == 1; }

  HAL_Bool HAL_CheckAnalogInputChannel(int32_t channel)
  {
    return channel < kNumAnalogInputs && channel >= 0;
  }

  void HAL_SetAnalogInputSimDevice(HAL_AnalogInputHandle handle,
                                   HAL_SimDeviceHandle device) {}

  void HAL_SetAnalogSampleRate(double samplesPerSecond, int32_t *status)
  {
    // TODO: This will change when variable size scan lists are implemented.
    // TODO: Need double comparison with epsilon.
    // wpi_assert(!sampleRateSet || GetSampleRate() == samplesPerSecond);
    initializeAnalog(status);
    if (*status != 0)
      return;
    setAnalogSampleRate(samplesPerSecond, status);
  }

  double HAL_GetAnalogSampleRate(int32_t *status)
  {
    initializeAnalog(status);
    if (*status != 0)
      return 0;
    uint32_t sampleRate = 1;
    *status = NiFpga_ReadU32(FPGASession, NiFpga_OpenSourceRIO_ControlU32_Analog_SampleRate, &sampleRate);
    return static_cast<double>(kTimebase) / static_cast<double>(sampleRate);
  }

  void HAL_SetAnalogAverageBits(HAL_AnalogInputHandle analogPortHandle,
                                int32_t bits, int32_t *status)
  {
    //Unsupported
  }

  int32_t HAL_GetAnalogAverageBits(HAL_AnalogInputHandle analogPortHandle,
                                   int32_t *status)
  {
    // Unsupported
    return 0;
  }

  void HAL_SetAnalogOversampleBits(HAL_AnalogInputHandle analogPortHandle,
                                   int32_t bits, int32_t *status)
  {
    //Unsupported
  }

  int32_t HAL_GetAnalogOversampleBits(HAL_AnalogInputHandle analogPortHandle,
                                      int32_t *status)
  {
    return 0;
  }

  int32_t HAL_GetAnalogValue(HAL_AnalogInputHandle analogPortHandle,
                             int32_t *status)
  {
    auto port = analogInputHandles->Get(analogPortHandle);
    if (port == nullptr)
    {
      *status = HAL_HANDLE_ERROR;
      return 0;
    }

    uint16_t value = 0;
    *status = NiFpga_ReadU16(FPGASession, port->control, &value);
    return value;
  }

  int32_t HAL_GetAnalogAverageValue(HAL_AnalogInputHandle analogPortHandle,
                                    int32_t *status)
  {
    auto port = analogInputHandles->Get(analogPortHandle);
    if (port == nullptr)
    {
      *status = HAL_HANDLE_ERROR;
      return 0;
    }

    // Right now, just return normal
    uint16_t value = 0;
    *status = NiFpga_ReadU16(FPGASession, port->control, &value);
    return value;
  }

  int32_t HAL_GetAnalogVoltsToValue(HAL_AnalogInputHandle analogPortHandle,
                                    double voltage, int32_t *status)
  {
    if (voltage > 5.0)
    {
      voltage = 5.0;
      *status = VOLTAGE_OUT_OF_RANGE;
    }
    if (voltage < 0.0)
    {
      voltage = 0.0;
      *status = VOLTAGE_OUT_OF_RANGE;
    }
    int32_t LSBWeight = HAL_GetAnalogLSBWeight(analogPortHandle, status);
    int32_t offset = HAL_GetAnalogOffset(analogPortHandle, status);
    int32_t value =
        static_cast<int32_t>((voltage + offset * 1.0e-9) / (LSBWeight * 1.0e-9));
    return value;
  }

  double HAL_GetAnalogVoltage(HAL_AnalogInputHandle analogPortHandle,
                              int32_t *status)
  {
    int32_t value = HAL_GetAnalogValue(analogPortHandle, status);
    int32_t LSBWeight = HAL_GetAnalogLSBWeight(analogPortHandle, status);
    //int32_t offset = HAL_GetAnalogOffset(analogPortHandle, status);

    //5 V ÷ 212

    // 0.001220703125

    double voltage = LSBWeight * 1.0e-9 * value;
    return voltage;
  }

  double HAL_GetAnalogValueToVolts(HAL_AnalogInputHandle analogPortHandle,
                                   int32_t rawValue, int32_t *status)
  {
    int32_t LSBWeight = HAL_GetAnalogLSBWeight(analogPortHandle, status);
    int32_t offset = HAL_GetAnalogOffset(analogPortHandle, status);
    double voltage = LSBWeight * 1.0e-9 * rawValue - offset * 1.0e-9;
    return voltage;
  }

  double HAL_GetAnalogAverageVoltage(HAL_AnalogInputHandle analogPortHandle,
                                     int32_t *status)
  {
    int32_t value = HAL_GetAnalogAverageValue(analogPortHandle, status);
    int32_t LSBWeight = HAL_GetAnalogLSBWeight(analogPortHandle, status);
    int32_t offset = HAL_GetAnalogOffset(analogPortHandle, status);
    int32_t oversampleBits =
        HAL_GetAnalogOversampleBits(analogPortHandle, status);
    double voltage =
        LSBWeight * 1.0e-9 * value / static_cast<double>(1 << oversampleBits) -
        offset * 1.0e-9;
    return voltage;
  }

  int32_t HAL_GetAnalogLSBWeight(HAL_AnalogInputHandle analogPortHandle,
                                 int32_t *status)
  {
    uint32_t lsb = 0;
    *status = NiFpga_ReadU32(FPGASession, NiFpga_OpenSourceRIO_IndicatorU32_Analog_LSBWeight, &lsb);
    return lsb;
  }

  int32_t HAL_GetAnalogOffset(HAL_AnalogInputHandle analogPortHandle,
                              int32_t *status)
  {
    int32_t offset = 0;
    *status = NiFpga_ReadI32(FPGASession, NiFpga_OpenSourceRIO_IndicatorI32_Analog_Offset, &offset);
    return offset;
  }
}
