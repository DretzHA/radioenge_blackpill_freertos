#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "radioenge_modem.h"
#include <stdio.h>

extern osTimerId_t PeriodicSendTimerHandle;
extern osThreadId_t AppSendTaskHandle;
extern ADC_HandleTypeDef hadc1;
extern osEventFlagsId_t ModemStatusFlagsHandle;
extern TIM_HandleTypeDef htim3;


void LoRaWAN_RxEventCallback(uint8_t *data, uint32_t length, uint32_t port, int32_t rssi, int32_t snr)
{

}

void PeriodicSendTimerCallback(void *argument)
{
}

void AppSendTaskCode(void *argument)
{
    /* USER CODE BEGIN 5 */
    /* Infinite loop */    
    uint32_t read;
    int32_t temp;
    uint8_t sendStr[20];

    while (1)
    {        
        
        LoRaWaitDutyCycle();   
        //write code to read from sensors and send via LoRaWAN
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 100);
        read = HAL_ADC_GetValue(&hadc1);
        temp = (int32_t)(33000 * ((float)read / 4096));
        sprintf(sendStr,"Temperature: %2d.%2d",temp/100,temp%100);
        LoRaSendNow(2,sendStr);
        osDelay(30000);
    }
}
