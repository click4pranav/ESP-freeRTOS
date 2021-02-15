#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "Arduino.h"
#include "FreeRTOS.h"


void GPIOVoltage_task(void *params);
void UART_task(void *params);

TaskHandle_t GPIOVoltage_task_handle = NULL;
TaskHandle_t UART_task_handle = NULL;
QueueHandle_t UART_queue_handle = NULL;
SemaphoreHandle_t xSync = NULL;

char user_msg[100];

void setup() {

  Serial.begin(115200);

  UART_queue_handle = xQueueCreate(1,sizeof(int));
  vSemaphoreCreateBinary(xSync);

	if(UART_queue_handle != NULL && xSync != NULL) {

		xTaskCreate(GPIOVoltage_task, "GPIOVoltage_task", 10000, NULL, 2, &GPIOVoltage_task_handle);
		xTaskCreate(UART_task, "UART_task", 10000, NULL, 3, &UART_task_handle);

	}
	else
	{

		sprintf((char*)user_msg,"\r\nQueue & Semaphore creation failed\r\n");
    	Serial.println(user_msg);

	}
}

void GPIOVoltage_task(void *params) {

	//xSemaphoreGive(xWork);

	while(1) {

		int adcVal = analogRead(34);

		if(xQueueSendToFront(UART_queue_handle, &adcVal, portMAX_DELAY) == pdTRUE) {

			sprintf(user_msg, "ADC Val: %d  ", adcVal);
            Serial.print(user_msg);
			delay(100);
			xSemaphoreGive(xSync);

			//traceISR_EXIT_TO_SCHEDULER();
		    //taskYIELD();

		}

		else {

			sprintf(user_msg, "Semaphore Key Not Given");
      		Serial.println(user_msg);
			delay(100);

		}

	}
}

void UART_task(void *params) {

	int received_adc;

	while(1) {

		if(xSemaphoreTake(xSync,portMAX_DELAY) == pdTRUE) {

			xQueueReceive(UART_queue_handle, &received_adc, 0);
			float voltage = (float) received_adc * ( 3.3/4096 );
			sprintf(user_msg, "Voltage: %d.%02u V", (int)voltage, (int) fabs(((voltage - (int)voltage) * 100)));
      		Serial.println(user_msg);
			delay(100);
		
		}
		else {

			sprintf(user_msg, "Semaphore Key not received.");
    		Serial.println(user_msg);
			delay(100);
			
		}
	}
}

void loop() {

}
