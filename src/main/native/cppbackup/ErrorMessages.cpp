#include "hal/HALBase.h"
#include "hal/Errors.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

#include "NiFpga.h"

#pragma GCC diagnostic pop

#include "hal/CAN.h"

extern "C" {
const char* HAL_GetErrorMessage(int32_t code) {
  switch (code) {
    case 0:
      return "";
    case NiFpga_Status_FifoTimeout:
      return NiFpga_Status_FifoTimeout_MESSAGE;
    case NiFpga_Status_TransferAborted:
      return NiFpga_Status_TransferAborted_MESSAGE;
    case NiFpga_Status_MemoryFull:
      return NiFpga_Status_MemoryFull_MESSAGE;
    case NiFpga_Status_SoftwareFault:
      return NiFpga_Status_SoftwareFault_MESSAGE;
    case NiFpga_Status_InvalidParameter:
      return NiFpga_Status_InvalidParameter_MESSAGE;
    case NiFpga_Status_ResourceNotFound:
      return NiFpga_Status_ResourceNotFound_MESSAGE;
    case NiFpga_Status_ResourceNotInitialized:
      return NiFpga_Status_ResourceNotInitialized_MESSAGE;
    case NiFpga_Status_HardwareFault:
      return NiFpga_Status_HardwareFault_MESSAGE;
    case NiFpga_Status_IrqTimeout:
      return NiFpga_Status_IrqTimeout_MESSAGE;
    case SAMPLE_RATE_TOO_HIGH:
      return SAMPLE_RATE_TOO_HIGH_MESSAGE;
    case VOLTAGE_OUT_OF_RANGE:
      return VOLTAGE_OUT_OF_RANGE_MESSAGE;
    case LOOP_TIMING_ERROR:
      return LOOP_TIMING_ERROR_MESSAGE;
    case SPI_WRITE_NO_MOSI:
      return SPI_WRITE_NO_MOSI_MESSAGE;
    case SPI_READ_NO_MISO:
      return SPI_READ_NO_MISO_MESSAGE;
    case SPI_READ_NO_DATA:
      return SPI_READ_NO_DATA_MESSAGE;
    case INCOMPATIBLE_STATE:
      return INCOMPATIBLE_STATE_MESSAGE;
    case NO_AVAILABLE_RESOURCES:
      return NO_AVAILABLE_RESOURCES_MESSAGE;
    case RESOURCE_IS_ALLOCATED:
      return RESOURCE_IS_ALLOCATED_MESSAGE;
    case RESOURCE_OUT_OF_RANGE:
      return RESOURCE_OUT_OF_RANGE_MESSAGE;
    case HAL_INVALID_ACCUMULATOR_CHANNEL:
      return HAL_INVALID_ACCUMULATOR_CHANNEL_MESSAGE;
    case HAL_HANDLE_ERROR:
      return HAL_HANDLE_ERROR_MESSAGE;
    case NULL_PARAMETER:
      return NULL_PARAMETER_MESSAGE;
    case ANALOG_TRIGGER_LIMIT_ORDER_ERROR:
      return ANALOG_TRIGGER_LIMIT_ORDER_ERROR_MESSAGE;
    case ANALOG_TRIGGER_PULSE_OUTPUT_ERROR:
      return ANALOG_TRIGGER_PULSE_OUTPUT_ERROR_MESSAGE;
    case PARAMETER_OUT_OF_RANGE:
      return PARAMETER_OUT_OF_RANGE_MESSAGE;
    case HAL_COUNTER_NOT_SUPPORTED:
      return HAL_COUNTER_NOT_SUPPORTED_MESSAGE;
    case HAL_ERR_CANSessionMux_InvalidBuffer:
      return ERR_CANSessionMux_InvalidBuffer_MESSAGE;
    case HAL_ERR_CANSessionMux_MessageNotFound:
      return ERR_CANSessionMux_MessageNotFound_MESSAGE;
    case HAL_WARN_CANSessionMux_NoToken:
      return WARN_CANSessionMux_NoToken_MESSAGE;
    case HAL_ERR_CANSessionMux_NotAllowed:
      return ERR_CANSessionMux_NotAllowed_MESSAGE;
    case HAL_ERR_CANSessionMux_NotInitialized:
      return ERR_CANSessionMux_NotInitialized_MESSAGE;
    // case VI_ERROR_SYSTEM_ERROR:
    //   return VI_ERROR_SYSTEM_ERROR_MESSAGE;
    // case VI_ERROR_INV_OBJECT:
    //   return VI_ERROR_INV_OBJECT_MESSAGE;
    // case VI_ERROR_RSRC_LOCKED:
    //   return VI_ERROR_RSRC_LOCKED_MESSAGE;
    // case VI_ERROR_RSRC_NFOUND:
    //   return VI_ERROR_RSRC_NFOUND_MESSAGE;
    // case VI_ERROR_INV_RSRC_NAME:
    //   return VI_ERROR_INV_RSRC_NAME_MESSAGE;
    // case VI_ERROR_QUEUE_OVERFLOW:
    //   return VI_ERROR_QUEUE_OVERFLOW_MESSAGE;
    // case VI_ERROR_IO:
    //   return VI_ERROR_IO_MESSAGE;
    // case VI_ERROR_ASRL_PARITY:
    //   return VI_ERROR_ASRL_PARITY_MESSAGE;
    // case VI_ERROR_ASRL_FRAMING:
    //   return VI_ERROR_ASRL_FRAMING_MESSAGE;
    // case VI_ERROR_ASRL_OVERRUN:
    //   return VI_ERROR_ASRL_OVERRUN_MESSAGE;
    // case VI_ERROR_RSRC_BUSY:
    //   return VI_ERROR_RSRC_BUSY_MESSAGE;
    // case VI_ERROR_INV_PARAMETER:
    //   return VI_ERROR_INV_PARAMETER_MESSAGE;
    case HAL_PWM_SCALE_ERROR:
      return HAL_PWM_SCALE_ERROR_MESSAGE;
    case HAL_SERIAL_PORT_NOT_FOUND:
      return HAL_SERIAL_PORT_NOT_FOUND_MESSAGE;
    case HAL_THREAD_PRIORITY_ERROR:
      return HAL_THREAD_PRIORITY_ERROR_MESSAGE;
    case HAL_THREAD_PRIORITY_RANGE_ERROR:
      return HAL_THREAD_PRIORITY_RANGE_ERROR_MESSAGE;
    case HAL_SERIAL_PORT_OPEN_ERROR:
      return HAL_SERIAL_PORT_OPEN_ERROR_MESSAGE;
    case HAL_SERIAL_PORT_ERROR:
      return HAL_SERIAL_PORT_ERROR_MESSAGE;
    case HAL_CAN_TIMEOUT:
      return HAL_CAN_TIMEOUT_MESSAGE;
    // case ERR_FRCSystem_NetCommNotResponding:
    //   return ERR_FRCSystem_NetCommNotResponding_MESSAGE;
    // case ERR_FRCSystem_NoDSConnection:
    //   return ERR_FRCSystem_NoDSConnection_MESSAGE;
    case HAL_CAN_BUFFER_OVERRUN:
      return HAL_CAN_BUFFER_OVERRUN_MESSAGE;
    case HAL_LED_CHANNEL_ERROR:
      return HAL_LED_CHANNEL_ERROR_MESSAGE;
    default:
      return "Unknown error status";
  }
}
}
