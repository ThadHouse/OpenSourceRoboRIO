#include "hal/HALBase.h"
#include "hal/DIO.h"
#include "hal/PWM.h"

#include "stdio.h"
#include "stdlib.h"


#include "NiFpga_OpenSourceRIO.h"

#include <unistd.h> // for usleep

void usleep(int val);

int main() {
  HAL_Bool didInit = HAL_Initialize(500, 0);

  if (!didInit) {
    printf("Failed to initialize\n");
    return 1;
  }

  int32_t status = 0;

  HAL_DigitalHandle pwm = HAL_InitializePWMPort(HAL_GetPort(0), &status);

  if (status != 0) {
    printf("Error: %d\n", status);
    return 1;
  }



  for(;;) {
    usleep(20 * 1000);
    status = 0;
    HAL_Bool button = HAL_GetFPGAButton(&status);
    //printf("Setting PWM %d\n", button);
    HAL_SetPWMRaw(pwm, button ? 2000 : 1000, &status);
    if (status != 0) {
      printf("Error: %d\n", status);
    }
  }
}
