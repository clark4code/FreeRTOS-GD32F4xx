#include "fd_uart.h"
#include <string.h>

#define FD_UART_EVT_TX_FINISH		BIT(0)
#define FD_UART_EVT_RX_FINISH		BIT(1)
#define FD_UART_EVT_ERROR			BIT(2)
#define FD_UART_EVT_END_ASYNC_RX	BIT(3)

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
	) {
	memset(self, 0, sizeof(fd_uart_t));
	self->usart_periph			= usart_periph;
	self->tx_dma_periph			= tx_dma_periph;
	self->tx_dma_channel		= tx_dma_channel;
	self->rx_dma_periph			= rx_dma_periph;
	self->rx_dma_channel		= rx_dma_channel;
	self->enable_tx_handler		= enable_tx_handler;

	// UART
	usart_deinit(usart_periph);
    usart_baudrate_set(usart_periph, baudrate);
	usart_dma_receive_config(usart_periph, USART_DENR_ENABLE);
    usart_dma_transmit_config(usart_periph, USART_DENT_ENABLE);
    usart_enable(usart_periph);

	// DMA TX
	dma_single_data_parameter_struct dma_init_struct;
	dma_single_data_para_struct_init(&dma_init_struct);
	dma_deinit(tx_dma_periph, tx_dma_channel);
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
	dma_init_struct.periph_addr = (uint32_t)&USART_DATA(usart_periph);
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
	dma_single_data_mode_init(tx_dma_periph, tx_dma_channel, &dma_init_struct);
	dma_circulation_disable(tx_dma_periph, tx_dma_channel);
	dma_channel_subperipheral_select(tx_dma_periph, tx_dma_channel, tx_dma_sub_periph);

	// DMA RX
	dma_deinit(rx_dma_periph, rx_dma_channel);
	dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.periph_addr = (uint32_t)&USART_DATA(usart_periph);
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
	dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
	dma_single_data_mode_init(rx_dma_periph, rx_dma_channel, &dma_init_struct);
	dma_circulation_disable(rx_dma_periph, rx_dma_channel);
	dma_channel_subperipheral_select(rx_dma_periph, rx_dma_channel, rx_dma_sub_periph);
	
	self->event_handle = xEventGroupCreate();
	xEventGroupSetBits(self->event_handle, FD_UART_EVT_TX_FINISH);
}

void fd_uart_irq(fd_uart_ptr self) {
#define IS_SET_UIF(b) usart_interrupt_flag_get(self->usart_periph, b) == SET
#define UIFC(f) usart_interrupt_flag_clear(self->usart_periph, f)
	uint32_t evt_bits = 0;
	if (IS_SET_UIF(USART_INT_FLAG_IDLE)) {
		usart_data_receive(self->usart_periph);	// 清除USART_INT_FLAG_IDLE
		uint32_t dma_count = dma_transfer_number_get(self->rx_dma_periph, self->rx_dma_channel);
		if (dma_count < self->rx_buffer_size) {
			self->rx_size = self->rx_buffer_size - dma_count;
		}
		usart_receive_config(self->usart_periph, USART_RECEIVE_DISABLE);
		dma_channel_disable(self->rx_dma_periph, self->rx_dma_channel);
		usart_interrupt_disable(self->usart_periph, USART_INT_IDLE);
		usart_interrupt_disable(self->usart_periph, USART_INT_ERR);
		evt_bits = FD_UART_EVT_RX_FINISH;
	} else if (IS_SET_UIF(USART_INT_FLAG_TC)) {
		UIFC(USART_INT_FLAG_TC );
		dma_channel_disable(self->tx_dma_periph, self->tx_dma_channel);
		usart_transmit_config(self->usart_periph, USART_TRANSMIT_DISABLE);
		if (self->enable_tx_handler) {
			self->enable_tx_handler(false);
		}
		evt_bits = FD_UART_EVT_TX_FINISH;
	} else if (IS_SET_UIF(USART_INT_FLAG_ERR_ORERR)
		|| IS_SET_UIF(USART_INT_FLAG_ERR_NERR)
		|| IS_SET_UIF(USART_INT_FLAG_ERR_FERR)) {
		usart_data_receive(self->usart_periph);
		usart_receive_config(self->usart_periph, USART_RECEIVE_DISABLE);
		dma_channel_disable(self->rx_dma_periph, self->rx_dma_channel);
		usart_interrupt_disable(self->usart_periph, USART_INT_IDLE);
		usart_interrupt_disable(self->usart_periph, USART_INT_ERR);
		evt_bits = FD_UART_EVT_ERROR;
	} else {
		UIFC(USART_INT_FLAG_CTS);
		UIFC(USART_INT_FLAG_LBD);
		UIFC(USART_INT_FLAG_RBNE);
		UIFC(USART_INT_FLAG_EB);
		UIFC(USART_INT_FLAG_RT);
	}
	if (evt_bits) {
		BaseType_t task_woken = pdFALSE;
		if(xEventGroupSetBitsFromISR(self->event_handle, evt_bits, &task_woken ) != pdFAIL ) {
			portYIELD_FROM_ISR( task_woken );
		}
	}
}

fc_error_t fd_uart_write(fd_uart_ptr self, const void *data, size_t size, TickType_t timeout) {
	EventBits_t evt_bits = xEventGroupWaitBits(self->event_handle, FD_UART_EVT_TX_FINISH, pdTRUE, pdFALSE, timeout);
	if ((evt_bits & FD_UART_EVT_TX_FINISH) == 0) {
		return FC_ERR_TIMEOUT;
	}
	if (self->enable_tx_handler) {
		self->enable_tx_handler(true);
	}
	dma_channel_disable(self->tx_dma_periph, self->tx_dma_channel);
	dma_interrupt_flag_clear(self->tx_dma_periph, self->tx_dma_channel, DMA_INT_FLAG_HTF);
	dma_interrupt_flag_clear(self->tx_dma_periph, self->tx_dma_channel, DMA_INT_FLAG_FTF);
	dma_interrupt_flag_clear(self->tx_dma_periph, self->tx_dma_channel, DMA_INT_FLAG_FEE);
	dma_interrupt_flag_clear(self->tx_dma_periph, self->tx_dma_channel, DMA_INT_FLAG_SDE);
	dma_interrupt_flag_clear(self->tx_dma_periph, self->tx_dma_channel, DMA_INT_FLAG_TAE);
	dma_interrupt_flag_clear(self->tx_dma_periph, self->tx_dma_channel, DMA_INT_FLAG_FTF);
	usart_transmit_config(self->usart_periph, USART_TRANSMIT_DISABLE);
	usart_interrupt_flag_clear(self->usart_periph, USART_INT_FLAG_TC);
	dma_memory_address_config( self->tx_dma_periph, self->tx_dma_channel, DMA_MEMORY_0, (uint32_t)data);
	dma_transfer_number_config(self->tx_dma_periph, self->tx_dma_channel, size);
	dma_channel_enable(self->tx_dma_periph, self->tx_dma_channel); 
	usart_transmit_config(self->usart_periph, USART_TRANSMIT_ENABLE);
	usart_interrupt_enable(self->usart_periph, USART_INT_TC);
	evt_bits = xEventGroupWaitBits(self->event_handle, FD_UART_EVT_TX_FINISH, pdFALSE, pdFALSE, timeout);
	return FC_OK;
}

size_t fd_uart_read(fd_uart_ptr self, void *data, size_t size, TickType_t timeout) {
	self->rx_buffer_size = size;
	self->rx_size = 0;
	dma_channel_disable(self->rx_dma_periph, self->rx_dma_channel);
	dma_interrupt_flag_clear(self->rx_dma_periph, self->rx_dma_channel, DMA_INT_FLAG_HTF);
	dma_interrupt_flag_clear(self->rx_dma_periph, self->rx_dma_channel, DMA_INT_FLAG_FTF);
	dma_interrupt_flag_clear(self->rx_dma_periph, self->rx_dma_channel, DMA_INT_FLAG_FEE);
	dma_interrupt_flag_clear(self->rx_dma_periph, self->rx_dma_channel, DMA_INT_FLAG_SDE);
	dma_interrupt_flag_clear(self->rx_dma_periph, self->rx_dma_channel, DMA_INT_FLAG_TAE);
	dma_interrupt_flag_clear(self->rx_dma_periph, self->rx_dma_channel, DMA_INT_FLAG_FTF);
	dma_memory_address_config( self->rx_dma_periph, self->rx_dma_channel, DMA_MEMORY_0, (uint32_t)data);
	dma_transfer_number_config(self->rx_dma_periph, self->rx_dma_channel, size);
	usart_interrupt_enable(self->usart_periph, USART_INT_IDLE);
	usart_interrupt_enable(self->usart_periph, USART_INT_ERR);
	xEventGroupClearBits(self->event_handle, FD_UART_EVT_RX_FINISH | FD_UART_EVT_ERROR);
	dma_channel_enable(self->rx_dma_periph, self->rx_dma_channel);
	usart_receive_config(self->usart_periph, USART_RECEIVE_ENABLE);
	EventBits_t evt_bits = xEventGroupWaitBits(self->event_handle, FD_UART_EVT_RX_FINISH | FD_UART_EVT_ERROR, pdTRUE, pdFALSE, timeout);
	dma_channel_disable(self->rx_dma_periph, self->rx_dma_channel);
	usart_receive_config(self->usart_periph, USART_RECEIVE_DISABLE);
	if ((evt_bits & FD_UART_EVT_RX_FINISH) == 0) {
		return 0;
	}
	return self->rx_size;
}

void fd_uart_read_handler(void *param) {
	fd_uart_ptr self = (fd_uart_ptr)param;
	size_t size;
	do {
		size = fd_uart_read(self, self->rx_buffer, self->rx_buffer_size, portMAX_DELAY);
		if (size > 0
			&& self->read_handler) {
			self->read_handler(self, self->rx_buffer, size);
		}
	} while (self->is_async_rx);
	xEventGroupSetBits(self->event_handle, FD_UART_EVT_END_ASYNC_RX);
	vTaskDelete(NULL);
}

void fd_uart_begin_read_async(fd_uart_ptr self, void *buffer, size_t buffer_size, fd_uart_read_handler_t read_handler) {
	if (self == NULL
		|| self->is_async_rx
		|| buffer == NULL
		|| buffer_size == 0
		|| read_handler == NULL) {
		return;
	}
	self->is_async_rx = true;
	self->rx_buffer_size = buffer_size;
	self->rx_buffer = buffer;
	self->read_handler = read_handler;
	xTaskCreate(fd_uart_read_handler, "uart_rx_handler", 128, NULL, 3, &self->rx_task_handler);
}

void fd_uart_end_read_async(fd_uart_ptr self) {
	if (self == NULL
		|| !self->is_async_rx) {
		return;
	}
	self->is_async_rx = false;
	xEventGroupSetBits(self->event_handle, FD_UART_EVT_TX_FINISH);
	xEventGroupWaitBits(self->event_handle, FD_UART_EVT_END_ASYNC_RX, pdTRUE, pdFALSE, pdMS_TO_TICKS(1000));
}