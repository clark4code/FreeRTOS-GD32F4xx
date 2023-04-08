#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H


#define configUSE_PREEMPTION		1	// 支持抢占
#define configUSE_IDLE_HOOK			0
#define configUSE_TICK_HOOK			0
#define configCPU_CLOCK_HZ			( ( unsigned long ) 168000000 )	// 系统时钟
#define configTICK_RATE_HZ			( ( TickType_t ) 1000 )			// Tick频率
#define configMAX_PRIORITIES		( 5 )							// 最大任务优先级
#define configMINIMAL_STACK_SIZE	( ( unsigned short ) 128 )		// 
#define configTOTAL_HEAP_SIZE		( ( size_t ) ( 44 * 1024 ) )	// 堆大小
#define configMAX_TASK_NAME_LEN		( 16 )							// 最大任务名长度
#define configUSE_TRACE_FACILITY	0
#define configUSE_16_BIT_TICKS		0	// TickType_t为32位
#define configIDLE_SHOULD_YIELD		1	// 
#define configUSE_TIMERS			1	
#define configTIMER_TASK_PRIORITY	4	// 系统定时任务优先级
#define	configTIMER_QUEUE_LENGTH	10	// 定时列长度
#define configTIMER_TASK_STACK_DEPTH	128
#define configCHECK_FOR_STACK_OVERFLOW	1	// 检查栈溢出，需自定义回调函数 void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
#define configASSERT(x)	if((x) == 0) { extern void fc_assert(const char *filename, int line); fc_assert(__FILE__, __LINE__); }	// 断言函数

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* 使能消息队列 */
#define configUSE_QUEUE_SETS    1
/* 使能信号量 */
#define configUSE_COUNTING_SEMAPHORES  1
/* 使能互斥信号量 */
#define  configUSE_MUTEXES  1

// 支持event
#define configSUPPORT_DYNAMIC_ALLOCATION	1

// 架构实际使用中断优先级位数
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS               __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS               4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY            0xf	// 最低中断优先级
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )	// 最低中断优先级在寄存器中的实际值

#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5	// FreeRTOS最大可管理的中断优先级，即只有优先级在 configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 和 configLIBRARY_LOWEST_INTERRUPT_PRIORITY 之间的中断可调用FreeRTOS的API
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )


/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete				1
#define INCLUDE_vTaskCleanUpResources	0
#define INCLUDE_vTaskSuspend			1
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay				1
#define INCLUDE_xTaskGetCurrentTaskHandle		1
#define INCLUDE_xEventGroupSetBitsFromISR	1
#define INCLUDE_xTimerPendFunctionCall	1


#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */

