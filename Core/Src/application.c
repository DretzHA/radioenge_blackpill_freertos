#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "radioenge_modem.h"
#include <stdio.h>
#include "uartRingBufDMA.h"
#include "main.h"

extern osTimerId_t PeriodicSendTimerHandle;
extern osThreadId_t AppSendTaskHandle;
extern ADC_HandleTypeDef hadc1;
extern osEventFlagsId_t ModemStatusFlagsHandle;
extern TIM_HandleTypeDef htim3;
extern osMessageQueueId_t compressorQueueHandle;
char msg[256];

typedef struct {
    uint32_t seq_no;
    int32_t temp_oCx100;   
} __attribute__((packed)) TEMPERATURE_OBJ_t;

typedef struct {
    uint16_t compressor_power;
    uint8_t warning_status;   
} __attribute__((packed)) COMPRESSOR_OBJ_t;


void compTaskCode(void *argument) {
    int16_t compressor_power;
    osStatus_t status;
    int16_t old_power = 1;
    char msg_compressor[256];
    while (1)
    {
        status = osMessageQueueGet(compressorQueueHandle, &compressor_power,
                                NULL, 20000); // wait for message
        if (status == osOK)
        {
            sprintf (msg, "Power: %d W\r\n", compressor_power);
            SendToUART(msg, strlen(msg));
            if (old_power == 1 && compressor_power==100) {
                for (int i=0;i<=100;i++) {
                htim3.Instance->CCR2 = (htim3.Instance->ARR*i)/100;
                sprintf (msg_compressor, "PWM: %d \r\n", i);
                SendToUART(msg_compressor, strlen(msg_compressor));
                }
            }
            else if (old_power == 100 && compressor_power==1) {
                for (int i=100;i>=0;i--) {
                htim3.Instance->CCR2 = (htim3.Instance->ARR*i)/100;
                sprintf (msg_compressor, "PWM: %d \r\n", i);
                SendToUART(msg_compressor, strlen(msg_compressor));
                }
            }
            else {
                sprintf (msg_compressor, "Mantendo status \r\n");
                SendToUART(msg_compressor, strlen(msg_compressor));
                }
        }
    }
            old_power = compressor_power;
            osDelay(200);
        }
        



void putCompressor(uint16_t *compressor_power) {
    osMessageQueuePut(compressorQueueHandle, &compressor_power, 0U, osWaitForever);
    osDelay(200);

}




void LoRaWAN_RxEventCallback(uint8_t *data, uint32_t length, uint32_t port, int32_t rssi, int32_t snr)
{
    COMPRESSOR_OBJ_t compressor;

    compressor.compressor_power = data[0];
    //compressor.warning_status = data[1];
    //printf(compressor.compressor_power);
    putCompressor(compressor.compressor_power);
}



void PeriodicSendTimerCallback(void *argument)
{
}




void AppSendTaskCode(void *argument)
{
    /* USER CODE BEGIN 5 */
    /* Infinite loop */    
    uint32_t read;
    // int16_t compressor_power;
    // osStatus_t status;
    // int32_t temp;
    // uint8_t sendStr[20];
    TEMPERATURE_OBJ_t temp;
    temp.seq_no = 0;
    while (1)
    {        
        
        LoRaWaitDutyCycle();   
        //write code to read from sensors and send via LoRaWAN
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 100);
        read = HAL_ADC_GetValue(&hadc1);
        temp.seq_no++;
        temp.temp_oCx100 = (int32_t)(330*((float)read/4096));
        LoRaSendBNow(2,(uint8_t*)&temp, sizeof(TEMPERATURE_OBJ_t));
        // temp = (int32_t)(33000 * ((float)read / 4096));
        // sprintf(sendStr,"Temperature: %02d.%02d",temp/100,temp%100);
        // LoRaSendNow(2,sendStr);
        // status = osMessageQueueGet(compressorQueueHandle, &compressor_power,
        //                         NULL, 20000); // wait for message
        // if (status == osOK)
        // {
        //     sprintf (msg, "Power: %d W\r\n", compressor_power);
        //     SendToUART(msg, strlen(msg));
        // }
        osDelay(30000);
    }
}
