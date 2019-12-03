/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2019 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "hal/PWM.h"

#include <cmath>
#include <thread>

#include <wpi/raw_ostream.h>

#include "ConstantsInternal.h"
#include "DigitalInternal.h"
#include "HALInitializer.h"
#include "PortsInternal.h"
#include "hal/cpp/fpga_clock.h"
#include "hal/handles/HandlesInternal.h"


#include "FPGA.h"



using namespace hal;

static inline int32_t GetMaxPositivePwm(DigitalPort* port) {
  return port->maxPwm;
}

static inline int32_t GetMinPositivePwm(DigitalPort* port) {
  if (port->eliminateDeadband) {
    return port->deadbandMaxPwm;
  } else {
    return port->centerPwm + 1;
  }
}

static inline int32_t GetCenterPwm(DigitalPort* port) {
  return port->centerPwm;
}

static inline int32_t GetMaxNegativePwm(DigitalPort* port) {
  if (port->eliminateDeadband) {
    return port->deadbandMinPwm;
  } else {
    return port->centerPwm - 1;
  }
}

static inline int32_t GetMinNegativePwm(DigitalPort* port) {
  return port->minPwm;
}

static inline int32_t GetPositiveScaleFactor(DigitalPort* port) {
  return GetMaxPositivePwm(port) - GetMinPositivePwm(port);
}  ///< The scale for positive speeds.

static inline int32_t GetNegativeScaleFactor(DigitalPort* port) {
  return GetMaxNegativePwm(port) - GetMinNegativePwm(port);
}  ///< The scale for negative speeds.

static inline int32_t GetFullRangeScaleFactor(DigitalPort* port) {
  return GetMaxPositivePwm(port) - GetMinNegativePwm(port);
}  ///< The scale for positions.

namespace hal {
namespace init {
void InitializePWM() {}
}  // namespace init
}  // namespace hal

extern "C" {

HAL_DigitalHandle HAL_InitializePWMPort(HAL_PortHandle portHandle,
                                        int32_t* status) {
  hal::init::CheckInit();
  initializeDigital(status);

  if (*status != 0) return HAL_kInvalidHandle;

  int16_t channel = getPortHandleChannel(portHandle);
  if (channel == InvalidHandleIndex || channel >= kNumPWMChannels) {
    *status = PARAMETER_OUT_OF_RANGE;
    return HAL_kInvalidHandle;
  }

  uint8_t origChannel = static_cast<uint8_t>(channel);

  channel += kNumDigitalChannels;  // remap Headers to end of allocations


  auto handle =
      digitalChannelHandles->Allocate(channel, HAL_HandleEnum::PWM, status);

  if (*status != 0)
    return HAL_kInvalidHandle;  // failed to allocate. Pass error back.

  auto port = digitalChannelHandles->Get(handle, HAL_HandleEnum::PWM);
  if (port == nullptr) {  // would only occur on thread issue.
    *status = HAL_HANDLE_ERROR;
    return HAL_kInvalidHandle;
  }

  port->channel = origChannel;

  // Defaults to allow an always valid config.
  HAL_SetPWMConfig(handle, 2.0, 1.501, 1.5, 1.499, 1.0, status);

  return handle;
}
void HAL_FreePWMPort(HAL_DigitalHandle pwmPortHandle, int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  digitalChannelHandles->Free(pwmPortHandle, HAL_HandleEnum::PWM);

  // Wait for no other object to hold this handle.
  auto start = hal::fpga_clock::now();
  while (port.use_count() != 1) {
    auto current = hal::fpga_clock::now();
    if (start + std::chrono::seconds(1) < current) {
      wpi::outs() << "PWM handle free timeout\n";
      wpi::outs().flush();
      break;
    }
    std::this_thread::yield();
  }
}

HAL_Bool HAL_CheckPWMChannel(int32_t channel) {
  return channel < kNumPWMChannels && channel >= 0;
}

void HAL_SetPWMConfig(HAL_DigitalHandle pwmPortHandle, double max,
                      double deadbandMax, double center, double deadbandMin,
                      double min, int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  // calculate the loop time in milliseconds
  double loopTime =
      HAL_GetPWMLoopTiming(status) / (kSystemClockTicksPerMicrosecond * 1e3);
  if (*status != 0) return;

  int32_t maxPwm = static_cast<int32_t>((max - kDefaultPwmCenter) / loopTime +
                                        kDefaultPwmStepsDown - 1);
  int32_t deadbandMaxPwm = static_cast<int32_t>(
      (deadbandMax - kDefaultPwmCenter) / loopTime + kDefaultPwmStepsDown - 1);
  int32_t centerPwm = static_cast<int32_t>(
      (center - kDefaultPwmCenter) / loopTime + kDefaultPwmStepsDown - 1);
  int32_t deadbandMinPwm = static_cast<int32_t>(
      (deadbandMin - kDefaultPwmCenter) / loopTime + kDefaultPwmStepsDown - 1);
  int32_t minPwm = static_cast<int32_t>((min - kDefaultPwmCenter) / loopTime +
                                        kDefaultPwmStepsDown - 1);

  port->maxPwm = maxPwm;
  port->deadbandMaxPwm = deadbandMaxPwm;
  port->deadbandMinPwm = deadbandMinPwm;
  port->centerPwm = centerPwm;
  port->minPwm = minPwm;
  port->configSet = true;
}

void HAL_SetPWMConfigRaw(HAL_DigitalHandle pwmPortHandle, int32_t maxPwm,
                         int32_t deadbandMaxPwm, int32_t centerPwm,
                         int32_t deadbandMinPwm, int32_t minPwm,
                         int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  port->maxPwm = maxPwm;
  port->deadbandMaxPwm = deadbandMaxPwm;
  port->deadbandMinPwm = deadbandMinPwm;
  port->centerPwm = centerPwm;
  port->minPwm = minPwm;
}

void HAL_GetPWMConfigRaw(HAL_DigitalHandle pwmPortHandle, int32_t* maxPwm,
                         int32_t* deadbandMaxPwm, int32_t* centerPwm,
                         int32_t* deadbandMinPwm, int32_t* minPwm,
                         int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  *maxPwm = port->maxPwm;
  *deadbandMaxPwm = port->deadbandMaxPwm;
  *deadbandMinPwm = port->deadbandMinPwm;
  *centerPwm = port->centerPwm;
  *minPwm = port->minPwm;
}

void HAL_SetPWMEliminateDeadband(HAL_DigitalHandle pwmPortHandle,
                                 HAL_Bool eliminateDeadband, int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  port->eliminateDeadband = eliminateDeadband;
}

HAL_Bool HAL_GetPWMEliminateDeadband(HAL_DigitalHandle pwmPortHandle,
                                     int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return false;
  }
  return port->eliminateDeadband;
}

void HAL_SetPWMRaw(HAL_DigitalHandle pwmPortHandle, int32_t value,
                   int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  if (value > 0xFFFF) {
    value = 0xFFFF;
  }
  if (value < 0) {
    value = 0;
  }


  *status = NiFpga_WriteU32(FPGASession, NiFpga_OpenSourceRIO_ControlU32_PWM_SetValue, (static_cast<uint32_t>(port->channel) << 16) | value);
}

void HAL_SetPWMSpeed(HAL_DigitalHandle pwmPortHandle, double speed,
                     int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  if (!port->configSet) {
    *status = INCOMPATIBLE_STATE;
    return;
  }

  DigitalPort* dPort = port.get();

  if (std::isfinite(speed)) {
    speed = std::clamp(speed, -1.0, 1.0);
  } else {
    speed = 0.0;
  }

  // calculate the desired output pwm value by scaling the speed appropriately
  int32_t rawValue;
  if (speed == 0.0) {
    rawValue = GetCenterPwm(dPort);
  } else if (speed > 0.0) {
    rawValue = static_cast<int32_t>(
        speed * static_cast<double>(GetPositiveScaleFactor(dPort)) +
        static_cast<double>(GetMinPositivePwm(dPort)) + 0.5);
  } else {
    rawValue = static_cast<int32_t>(
        speed * static_cast<double>(GetNegativeScaleFactor(dPort)) +
        static_cast<double>(GetMaxNegativePwm(dPort)) + 0.5);
  }

  if (!((rawValue >= GetMinNegativePwm(dPort)) &&
        (rawValue <= GetMaxPositivePwm(dPort))) ||
      rawValue == kPwmDisabled) {
    *status = HAL_PWM_SCALE_ERROR;
    return;
  }

  HAL_SetPWMRaw(pwmPortHandle, rawValue, status);
}

void HAL_SetPWMPosition(HAL_DigitalHandle pwmPortHandle, double pos,
                        int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }
  if (!port->configSet) {
    *status = INCOMPATIBLE_STATE;
    return;
  }
  DigitalPort* dPort = port.get();

  if (pos < 0.0) {
    pos = 0.0;
  } else if (pos > 1.0) {
    pos = 1.0;
  }

  // note, need to perform the multiplication below as floating point before
  // converting to int
  int32_t rawValue = static_cast<int32_t>(
      (pos * static_cast<double>(GetFullRangeScaleFactor(dPort))) +
      GetMinNegativePwm(dPort));

  if (rawValue == kPwmDisabled) {
    *status = HAL_PWM_SCALE_ERROR;
    return;
  }

  HAL_SetPWMRaw(pwmPortHandle, rawValue, status);
}

void HAL_SetPWMDisabled(HAL_DigitalHandle pwmPortHandle, int32_t* status) {
  HAL_SetPWMRaw(pwmPortHandle, kPwmDisabled, status);
}

int32_t HAL_GetPWMRaw(HAL_DigitalHandle pwmPortHandle, int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }

  uint16_t arr[NiFpga_OpenSourceRIO_IndicatorArrayU16Size_PWM_Values];
  *status = NiFpga_ReadArrayU16(FPGASession, NiFpga_OpenSourceRIO_IndicatorArrayU16_PWM_Values, arr, NiFpga_OpenSourceRIO_IndicatorArrayU16Size_PWM_Values);

  return arr[port->channel];
}

double HAL_GetPWMSpeed(HAL_DigitalHandle pwmPortHandle, int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }
  if (!port->configSet) {
    *status = INCOMPATIBLE_STATE;
    return 0;
  }

  int32_t value = HAL_GetPWMRaw(pwmPortHandle, status);
  if (*status != 0) return 0;
  DigitalPort* dPort = port.get();

  if (value == kPwmDisabled) {
    return 0.0;
  } else if (value > GetMaxPositivePwm(dPort)) {
    return 1.0;
  } else if (value < GetMinNegativePwm(dPort)) {
    return -1.0;
  } else if (value > GetMinPositivePwm(dPort)) {
    return static_cast<double>(value - GetMinPositivePwm(dPort)) /
           static_cast<double>(GetPositiveScaleFactor(dPort));
  } else if (value < GetMaxNegativePwm(dPort)) {
    return static_cast<double>(value - GetMaxNegativePwm(dPort)) /
           static_cast<double>(GetNegativeScaleFactor(dPort));
  } else {
    return 0.0;
  }
}

double HAL_GetPWMPosition(HAL_DigitalHandle pwmPortHandle, int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }
  if (!port->configSet) {
    *status = INCOMPATIBLE_STATE;
    return 0;
  }

  int32_t value = HAL_GetPWMRaw(pwmPortHandle, status);
  if (*status != 0) return 0;
  DigitalPort* dPort = port.get();

  if (value < GetMinNegativePwm(dPort)) {
    return 0.0;
  } else if (value > GetMaxPositivePwm(dPort)) {
    return 1.0;
  } else {
    return static_cast<double>(value - GetMinNegativePwm(dPort)) /
           static_cast<double>(GetFullRangeScaleFactor(dPort));
  }
}

void HAL_LatchPWMZero(HAL_DigitalHandle pwmPortHandle, int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  // pwmSystem->writeZeroLatch(port->channel, true, status);
  // pwmSystem->writeZeroLatch(port->channel, false, status);
}

void HAL_SetPWMPeriodScale(HAL_DigitalHandle pwmPortHandle, int32_t squelchMask,
                           int32_t* status) {
  auto port = digitalChannelHandles->Get(pwmPortHandle, HAL_HandleEnum::PWM);
  if (port == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  *status = NiFpga_WriteU16(FPGASession, NiFpga_OpenSourceRIO_ControlU16_PWM_SetPeriodScale, (static_cast<uint32_t>(port->channel) << 8) | squelchMask);

  // if (port->channel < tPWM::kNumPeriodScaleHdrElements) {
  //   pwmSystem->writePeriodScaleHdr(port->channel, squelchMask, status);
  // } else {
  //   pwmSystem->writePeriodScaleMXP(
  //       port->channel - tPWM::kNumPeriodScaleHdrElements, squelchMask, status);
  // }
}

int32_t HAL_GetPWMLoopTiming(int32_t* status) {
  initializeDigital(status);
  if (*status != 0) return 0;
  return 40;
}

uint64_t HAL_GetPWMCycleStartTime(int32_t* status) {
  initializeDigital(status);
  if (*status != 0) return 0;

  uint32_t upper1 = 0;
  uint32_t lower = 0;
  uint32_t upper2 = 0;
  *status = NiFpga_ReadU32(FPGASession, NiFpga_OpenSourceRIO_IndicatorU32_PWM_Timing_Upper, &upper1);
  *status = NiFpga_ReadU32(FPGASession, NiFpga_OpenSourceRIO_IndicatorU32_PWM_Timing_Lower, &lower);
  *status = NiFpga_ReadU32(FPGASession, NiFpga_OpenSourceRIO_IndicatorU32_PWM_Timing_Upper, &upper2);
  if (NiFpga_IsError(*status)) {
    return 0;
  }

  if (upper1 != upper2) {
    *status = NiFpga_ReadU32(FPGASession, NiFpga_OpenSourceRIO_IndicatorU32_PWM_Timing_Lower, &lower);
    if (NiFpga_IsError(*status)) {
      return 0;
    }
  }
  return (((uint64_t)upper2) << 32) + lower;
}

}  // extern "C"
