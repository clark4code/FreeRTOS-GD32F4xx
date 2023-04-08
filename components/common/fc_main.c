
#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

extern void app_main(void *pvParameters);

int main(void) {
	nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);	// 所有中断优先级位都为优先级组位，即都为抢占优先级位，FreeRTOS不支持子优先级
   	xTaskCreate(app_main, "app_main", 128, NULL, 1, NULL);
	vTaskStartScheduler();
	return 0;
}

void fc_assert(const char *filename, int line) {
	(void)filename;
	(void)line;
	// volatile int l = line;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
	(void)xTask;
	(void)pcTaskName;
	// volatile char* l = pcTaskName;
}