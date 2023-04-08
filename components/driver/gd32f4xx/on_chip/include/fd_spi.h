#pragma once
#include "gd32f4xx.h"
#include <stdbool.h>
#include <FreeRTOS.h>
#include <event_groups.h>
#include "fc_types.h"

typedef struct fd_spi_t {
	uint32_t			spi_periph;
	uint32_t 			rx_dma_periph;
	dma_channel_enum	rx_dma_channel;
	uint32_t 			tx_dma_periph;
	dma_channel_enum	tx_dma_channel;
	EventGroupHandle_t	event_handle;
} fd_spi_t, *fd_spi_ptr;

void fd_spi_init(
	fd_spi_ptr				self,
	uint32_t				spi_periph,				// SPIx
	uint32_t				clock_polarity_phase,	// SPI_CK_PL_xx
	uint32_t 				prescale,				// SPI_PSC_xx, PCLK/xx
	uint32_t 				tx_dma_periph, 
	dma_channel_enum		tx_dma_channel,
	dma_subperipheral_enum	tx_dma_sub_periph,
	uint32_t 				rx_dma_periph, 
	dma_channel_enum		rx_dma_channel, 
	dma_subperipheral_enum	rx_dma_sub_periph
	);
fc_error_t fd_spi_tr(fd_spi_ptr self, const void *txd, void *rxd, size_t size, TickType_t timeout);
void fd_spi_irq(fd_spi_ptr self);
