#include "hal/Interrupts.h"


#include "memory"

#include "DigitalInternal.h"
#include "HALInitializer.h"
#include "PortsInternal.h"
#include "hal/Errors.h"
#include "hal/handles/HandlesInternal.h"
#include "hal/handles/LimitedHandleResource.h"

#include "NiFpga_OpenSourceRIO.h"
#include "Globals.h"

using namespace hal::internal;

using namespace hal;

namespace {
struct Interrupt {
  NiFpga_Irq irqs;
  NiFpga_IrqContext context;
  void* param;
  uint32_t channel;
  uint32_t risingChannel;
  uint32_t fallingChannel;
};
}

static LimitedHandleResource<HAL_InterruptHandle, Interrupt, kNumInterrupts,
                             HAL_HandleEnum::Interrupt>* interruptHandles;

namespace hal {
namespace init {
void InitialzeInterrupts() {
  static LimitedHandleResource<HAL_InterruptHandle, Interrupt, kNumInterrupts,
                               HAL_HandleEnum::Interrupt>
      iH;
  interruptHandles = &iH;
}
}  // namespace init
}  // namespace hal

HAL_InterruptHandle HAL_InitializeInterrupts(HAL_Bool watcher,
                                             int32_t* status) {
  hal::init::CheckInit();
  HAL_InterruptHandle handle = interruptHandles->Allocate();
  if (handle == HAL_kInvalidHandle) {
    *status = NO_AVAILABLE_RESOURCES;
    return HAL_kInvalidHandle;
  }
  auto anInterrupt = interruptHandles->Get(handle);
  //uint32_t interruptIndex = static_cast<uint32_t>(getHandleIndex(handle));

  *status = NiFpga_ReserveIrqContext(FPGASession, &anInterrupt->context);

  if (NiFpga_IsError(*status)) {
    interruptHandles->Free(handle);
    return HAL_kInvalidHandle;
  }

  anInterrupt->channel = NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_Resource;
  anInterrupt->risingChannel = NiFpga_OpenSourceRIO_IndicatorU32_Interrupt0_RisingTimestamp;
  anInterrupt->fallingChannel = NiFpga_OpenSourceRIO_IndicatorU32_Interrupt0_FallingTimestamp;

  anInterrupt->irqs = static_cast<NiFpga_Irq>(NiFpga_Irq_0 | NiFpga_Irq_8);
  return handle;
}

void* HAL_CleanInterrupts(HAL_InterruptHandle interruptHandle,
                          int32_t* status) {
  auto anInterrupt = interruptHandles->Get(interruptHandle);
  interruptHandles->Free(interruptHandle);
  if (anInterrupt == nullptr) {
    return nullptr;
  }
  *status = NiFpga_UnreserveIrqContext(FPGASession, anInterrupt->context);
  void* param = anInterrupt->param;
  return param;
}

int64_t HAL_WaitForInterrupt(HAL_InterruptHandle interruptHandle,
                             double timeout, HAL_Bool ignorePrevious,
                             int32_t* status) {
  auto anInterrupt = interruptHandles->Get(interruptHandle);
  if (anInterrupt == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }

  NiFpga_IrqContext context;
  *status = NiFpga_ReserveIrqContext(FPGASession, &context);
  if (NiFpga_IsError(*status)) {
    return 0;
  }

  if (ignorePrevious) {
    NiFpga_AcknowledgeIrqs(FPGASession, anInterrupt->irqs);
  }

  uint32_t assertedIrqs = 0;
  NiFpga_Bool timedOut = 0;
  *status = NiFpga_WaitOnIrqs(FPGASession, context, anInterrupt->irqs, static_cast<uint32_t>(timeout * 1e3), &assertedIrqs, &timedOut);


  // Don't report a timeout as an error - the return code is enough to tell
  // that a timeout happened.
  if (timedOut) {
    *status = NiFpga_Status_Success;
  } else {
    NiFpga_AcknowledgeIrqs(FPGASession, assertedIrqs);
  }
  return assertedIrqs;
}

int64_t HAL_ReadInterruptRisingTimestamp(HAL_InterruptHandle interruptHandle,
                                         int32_t* status) {
  auto anInterrupt = interruptHandles->Get(interruptHandle);
  if (anInterrupt == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }
  uint32_t timestamp = 0;
  *status = NiFpga_ReadU32(FPGASession, anInterrupt->risingChannel, &timestamp);
  return timestamp;
}

int64_t HAL_ReadInterruptFallingTimestamp(HAL_InterruptHandle interruptHandle,
                                          int32_t* status) {
  auto anInterrupt = interruptHandles->Get(interruptHandle);
  if (anInterrupt == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return 0;
  }
  uint32_t timestamp = 0;
  *status = NiFpga_ReadU32(FPGASession, anInterrupt->fallingChannel, &timestamp);
  return timestamp;
}

void HAL_RequestInterrupts(HAL_InterruptHandle interruptHandle,
                           HAL_Handle digitalSourceHandle,
                           HAL_AnalogTriggerType analogTriggerType,
                           int32_t* status) {
  auto anInterrupt = interruptHandles->Get(interruptHandle);
  if (anInterrupt == nullptr) {
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

  uint8_t data[NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_PackedSizeInBytes];
  NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_Type configType;
  NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_UnpackCluster(data, &configType);

  configType.Source.AnalogTrigger = routingAnalogTrigger;
  configType.Source.Channel = routingChannel;
  configType.Source.Module = routingModule;

  NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_PackCluster(data, &configType);
  *status = NiFpga_WriteArrayU8(FPGASession, anInterrupt->channel, data, NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_PackedSizeInBytes);
}

void HAL_SetInterruptUpSourceEdge(HAL_InterruptHandle interruptHandle,
                                  HAL_Bool risingEdge, HAL_Bool fallingEdge,
                                  int32_t* status) {
  auto anInterrupt = interruptHandles->Get(interruptHandle);
  if (anInterrupt == nullptr) {
    *status = HAL_HANDLE_ERROR;
    return;
  }

  uint8_t data[NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_PackedSizeInBytes];
  *status = NiFpga_ReadArrayU8(FPGASession, anInterrupt->channel, data, NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_PackedSizeInBytes);

  if (NiFpga_IsError(*status)) {
    return;
  }

  NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_Type configType;
  NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_UnpackCluster(data, &configType);

  configType.RisingEdge = risingEdge;
  configType.FallingEdge =  fallingEdge;

  NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_PackCluster(data, &configType);
  *status = NiFpga_WriteArrayU8(FPGASession, anInterrupt->channel, data, NiFpga_OpenSourceRIO_ControlCluster_Interrupt0_Config_PackedSizeInBytes);


}
