#pragma once
#include "gd32f4xx.h"
#include <stdbool.h>
#include <FreeRTOS.h>
#include <event_groups.h>
#include <fc_types.h>

struct fd_uart_t;
typedef void (*fd_uart_enable_handler_t)(bool enable);
typedef void (*fd_uart_read_handler_t)(struct fd_uart_t *self, const uint8_t *data, size_t size);

typedef struct fd_uart_t {
	uint32_t					usart_periph;
	void*						rx_buffer;
	size_t						rx_buffer_size;
	size_t						rx_size;
	uint32_t 					rx_dma_periph;
	dma_channel_enum			rx_dma_channel;
	uint32_t 					tx_dma_periph;
	dma_channel_enum			tx_dma_channel;	
	TaskHandle_t				rx_task_handler;
	bool						is_async_rx;
	fd_uart_enable_handler_t	enable_tx_handler;
	fd_uart_read_handler_t		read_handler;
	EventGroupHandle_t			event_handle;	
} fd_uart_t, *fd_uart_ptr;

void fd_uart_init(
	fd_uart_ptr				self,				
	uint32_t					baudrate,
	uint32_t					usart_periph,
	uint32_t 					tx_dma_periph, 
	dma_channel_enum			tx_dma_channel,
	dma_subperipheral_enum		tx_dma_sub_periph,
	uint32_t 					rx_dma_periph, 
	dma_channel_enum			rx_dma_channel, 
	dma_subperipheral_enum		rx_dma_sub_periph,
	fd_uart_enable_handler_t	enable_tx_handler
	);

void fd_uart_irq(fd_uart_ptr self);

fc_error_t fd_uart_write(fd_uart_ptr self, const void *data, size_t size, TickType_t timeout);

size_t fd_uart_read(fd_uart_ptr self, void *data, size_t size, TickType_t timeout);

void fd_uart_begin_read_async(fd_uart_ptr self, void *buffer, size_t buffer_size, fd_uart_read_handler_t read_handler);

void fd_uart_end_read_async(fd_uart_ptr self);