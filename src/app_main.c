
#include "gd32f4xx.h"
#include "FreeRTOS.h"

void app_main(void* param) {
	(void)param;
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
