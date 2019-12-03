#include "hal/HALBase.h"
#include "hal/DIO.h"
#include "hal/PWM.h"
#include "hal/Notifier.h"

#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"

#include "ntcore_c.h"

#include "hal/AnalogInput.h"
#include "hal/AnalogOutput.h"
#include "hal/Accelerometer.h"
#include "hal/Interrupts.h"

#include <unistd.h> // for usleep

#include "string.h"

void usleep(int val);

void* notifierMain(void* param) {
  int32_t status = 0;
  HAL_NotifierHandle notifier = HAL_InitializeNotifier(&status);
  HAL_UpdateNotifierAlarm(notifier, HAL_GetFPGATime(&status) + 1000 * 500, &status);
  while (1) {
    uint64_t time = HAL_WaitForNotifierAlarm(notifier, &status);
    //printf("Notifier!\n");
    HAL_UpdateNotifierAlarm(notifier, time + 1000 * 500, &status);
  }
  return NULL;
}



void* interruptMain(void* param) {
  int32_t status = 0;
  HAL_InterruptHandle interrupt = HAL_InitializeInterrupts(0, &status);

  printf("Create Interrupt %d\n", status);

  HAL_DigitalHandle input = *(HAL_DigitalHandle*)param;

  HAL_SetInterruptUpSourceEdge(interrupt, 0, 1, &status);

  printf("Up Source Edge %d\n", status);

  HAL_RequestInterrupts(interrupt, input, HAL_Trigger_kFallingPulse, &status);

  printf("Request %d\n", status);

  while (1) {
    int64_t mask = HAL_WaitForInterrupt(interrupt, 1, 0, &status);

    printf("Wait %d\n", status);
    if (mask == 0) {
      printf("Timeout!\n");
    } else {
      printf("Interrupt! %d\n", (int)mask);
      printf("Time %d\n", (int)HAL_ReadInterruptFallingTimestamp(interrupt, &status));
    }
  }
}

int main() {
  HAL_Bool didInit = HAL_Initialize(500, 0);

  if (!didInit) {
    printf("Failed to initialize\n");
    return 1;
  }

  pthread_t thread;
  pthread_create(&thread, NULL, notifierMain, NULL);



  int32_t status = 0;

  HAL_DigitalHandle pwm = HAL_InitializePWMPort(HAL_GetPort(0), &status);

  if (status != 0) {
    printf("Error: %d\n", status);
    return 1;
  }

  HAL_DigitalHandle input = HAL_InitializeDIOPort(HAL_GetPort(0), 1, &status);

      pthread_t interruptThread;
  pthread_create(&interruptThread, NULL, interruptMain, &input);

  HAL_AnalogInputHandle ai = HAL_InitializeAnalogInputPort(HAL_GetPort(0), &status);

  HAL_AnalogOutputHandle ao = HAL_InitializeAnalogOutputPort(HAL_GetPort(0), &status);

  printf("LSB %d\n", HAL_GetAnalogLSBWeight(ai, &status));
  printf("Offset %d\n", HAL_GetAnalogOffset(ai, &status));

  NT_Inst inst = NT_GetDefaultInstance();

  NT_StartServer(inst, "PersistFile.ini", "", 1735);

  NT_Entry inEntry = NT_GetEntry(inst, "VoltageIn", strlen("VoltageIn"));

  NT_SetEntryDouble(inEntry, 0, 0, 1);

  NT_Entry outEntry = NT_GetEntry(inst, "VoltageOut", strlen("VoltageOut"));

  NT_Entry outEntry2 = NT_GetEntry(inst, "VoltageOut2", strlen("VoltageOut2"));

  NT_Entry outEntry3 = NT_GetEntry(inst, "VoltageOut3", strlen("VoltageOut3"));

  NT_Entry inputEntry = NT_GetEntry(inst, "Input", strlen("Input"));

  uint64_t ts = 0;

  for(;;) {
    usleep(20 * 1000);

    status = 0;
    HAL_Bool button = HAL_GetFPGAButton(&status);

    double val = 0;
    NT_GetEntryDouble(inEntry, &ts, &val);
    HAL_SetAnalogOutput(ao, val, &status);

    NT_SetEntryDouble(outEntry, 0, HAL_GetAnalogVoltage(ai, &status), 1);

    NT_SetEntryDouble(outEntry2, 0, HAL_GetAnalogValue(ai, &status), 1);

    NT_SetEntryDouble(outEntry3, 0, HAL_GetAnalogValue(ai, &status) * 0.001220703125, 1);

    NT_SetEntryBoolean(inputEntry, 0, HAL_GetDIO(input, &status), 1);

    //printf("Setting PWM %d\n", button);
    HAL_SetPWMRaw(pwm, button ? 2000 : 1000, &status);
    if (status != 0) {
      printf("Error: %d\n", status);
    }
  }
}
