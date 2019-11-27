#pragma once

#include "hal/Types.h"

#define HAL_Handle_InvalidHandleIndex -1

enum HAL_HandleEnum {
  HAL_HandleEnum_HAL_HandleEnum_Undefined = 0,
  HAL_HandleEnum_DIO = 1,
  HAL_HandleEnum_Port = 2,
  HAL_HandleEnum_Notifier = 3,
  HAL_HandleEnum_Interrupt = 4,
  HAL_HandleEnum_AnalogOutput = 5,
  HAL_HandleEnum_AnalogInput = 6,
  HAL_HandleEnum_AnalogTrigger = 7,
  HAL_HandleEnum_Relay = 8,
  HAL_HandleEnum_PWM = 9,
  HAL_HandleEnum_DigitalPWM = 10,
  HAL_HandleEnum_Counter = 11,
  HAL_HandleEnum_FPGAEncoder = 12,
  HAL_HandleEnum_Encoder = 13,
  HAL_HandleEnum_Compressor = 14,
  HAL_HandleEnum_Solenoid = 15,
  HAL_HandleEnum_AnalogGyro = 16,
  HAL_HandleEnum_Vendor = 17,
  HAL_HandleEnum_SimulationJni = 18,
  HAL_HandleEnum_CAN = 19,
  HAL_HandleEnum_SerialPort = 20,
  HAL_HandleEnum_DutyCycle = 21,
  HAL_HandleEnum_DMA = 22,
  HAL_HandleEnum_AddressableLED = 23,
};

/**
 * Get the handle index from a handle.
 *
 * @param handle the handle
 * @return       the index
 */
static inline int16_t getHandleIndex(HAL_Handle handle) {
  // mask and return last 16 bits
  return (int16_t)(handle & 0xffff);
}

/**
 * Get the handle type from a handle.
 *
 * @param handle the handle
 * @return       the type
 */
static inline enum HAL_HandleEnum getHandleType(HAL_Handle handle) {
  // mask first 8 bits and cast to enum
  return ((handle >> 24) & 0xff);
}

/**
 * Get if the handle is a specific type.
 *
 * @param handle     the handle
 * @param handleType the type to check
 * @return           true if the type is correct, otherwise false
 */
static inline HAL_Bool isHandleType(HAL_Handle handle, enum HAL_HandleEnum handleType) {
  return handleType == getHandleType(handle);
}

/**
 * Get if the version of the handle is correct.
 *
 * Do not use on the roboRIO, used specifically for the sim to handle resets.
 *
 * @param handle  the handle
 * @param version the handle version to check
 * @return        true if the handle is the right version, otherwise false
 */
static inline HAL_Bool isHandleCorrectVersion(HAL_Handle handle, int16_t version) {
  return (((handle & 0xFF0000) >> 16) & version) == version;
}

/**
 * Get if the handle is a correct type and version.
 *
 * Note the version is not checked on the roboRIO.
 *
 * @param handle     the handle
 * @param handleType the type to check
 * @param version    the handle version to check
 * @return           true if the handle is proper version and type, otherwise
 * false.
 */
static inline int16_t getHandleTypedIndex(HAL_Handle handle,
                                          enum HAL_HandleEnum enumType,
                                          int16_t version) {
  if (!isHandleType(handle, enumType)) return HAL_Handle_InvalidHandleIndex;
#if !defined(__FRC_ROBORIO__)
  if (!isHandleCorrectVersion(handle, version)) return HAL_Handle_InvalidHandleIndex;
#endif
  return getHandleIndex(handle);
}

/* specialized functions for Port handle
 * Port Handle Data Layout
 * Bits 0-7:   Channel Number
 * Bits 8-15:  Module Number
 * Bits 16-23: Unused
 * Bits 24-30: Handle Type
 * Bit 31:     1 if handle error, 0 if no error
 */

// using a 16 bit value so we can store 0-255 and still report error
/**
 * Gets the port channel of a port handle.
 *
 * @param handle the port handle
 * @return       the port channel
 */
static inline int16_t getPortHandleChannel(HAL_PortHandle handle) {
  if (!isHandleType(handle, HAL_HandleEnum_Port)) return HAL_Handle_InvalidHandleIndex;
  return (handle & 0xff);
}

// using a 16 bit value so we can store 0-255 and still report error
/**
 * Gets the port module of a port handle.
 *
 * @param handle the port handle
 * @return       the port module
 */
static inline int16_t getPortHandleModule(HAL_PortHandle handle) {
  if (!isHandleType(handle, HAL_HandleEnum_Port)) return HAL_Handle_InvalidHandleIndex;
  return ((handle >> 8) & 0xff);
}

// using a 16 bit value so we can store 0-255 and still report error
/**
 * Gets the SPI channel of a port handle.
 *
 * @param handle the port handle
 * @return       the port SPI channel
 */
static inline int16_t getPortHandleSPIEnable(HAL_PortHandle handle) {
  if (!isHandleType(handle, HAL_HandleEnum_Port)) return HAL_Handle_InvalidHandleIndex;
  return ((handle >> 16) & 0xff);
}


HAL_PortHandle HAL_Handle_CreatePortHandle(uint8_t channel, uint8_t module);

HAL_Handle HAL_Handle_CreateHandle(int16_t index, enum HAL_HandleEnum handleType,
                        int16_t version);
