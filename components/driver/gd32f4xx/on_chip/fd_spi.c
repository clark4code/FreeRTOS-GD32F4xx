#include "fd_spi.h"
#include <string.h>

#define FD_SPI_EVT_FINISH	BIT(0)

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
	) {
	memset(self, 0, sizeof(fd_spi_t));
	self->spi_periph 			= spi_periph;
	self->tx_dma_periph			= tx_dma_periph;
	self->tx_dma_channel		= tx_dma_channel;
	self->rx_dma_periph			= rx_dma_periph;
	self->rx_dma_channel		= rx_dma_channel;

	

	// SPI
	spi_parameter_struct spi_init_struct;
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = clock_polarity_phase;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = prescale;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(spi_periph, &spi_init_struct);
	spi_enable(spi_periph);

	// DMA TX
	dma_single_data_parameter_struct dma_init_struct;
	dma_single_data_para_struct_init(&dma_init_struct);
    dma_deinit(tx_dma_periph, tx_dma_channel);
    dma_init_struct.periph_addr         = (uint32_t)&SPI_DATA(spi_periph);
    dma_init_struct.direction           = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority            = DMA_PRIORITY_LOW;
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_DISABLE;
    dma_single_data_mode_init(tx_dma_periph, tx_dma_channel, &dma_init_struct);
    dma_channel_subperipheral_select(tx_dma_periph, tx_dma_channel, tx_dma_sub_periph);
	
    // DMA RX
	dma_deinit(rx_dma_periph, rx_dma_channel);
	dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(spi_periph);
	dma_init_struct.memory0_addr = 0;
	dma_init_struct.number       = 0;
	dma_init_struct.direction    = DMA_PERIPH_TO_MEMORY;
	dma_init_struct.priority     = DMA_PRIORITY_HIGH;
	dma_single_data_mode_init(rx_dma_periph, rx_dma_channel, &dma_init_struct);
	dma_channel_subperipheral_select(rx_dma_periph, rx_dma_channel, rx_dma_sub_periph);

	// event
	self->event_handle = xEventGroupCreate();
}


fc_error_t fd_spi_tr(fd_spi_ptr self, const void *txd, void *rxd, size_t size, TickType_t timeout) {
	if ((txd == NULL
			&& rxd == NULL)
		|| size == 0) {
		return FC_ERR_INVALID_PARAM;
	}
	if (spi_i2s_flag_get(self->spi_periph, SPI_FLAG_RBNE)) {
		spi_i2s_data_receive(self->spi_periph);
	}
	dma_channel_disable(self->tx_dma_periph, self->tx_dma_channel);
	dma_flag_clear(self->tx_dma_periph, self->tx_dma_channel, DMA_FLAG_FEE);
	dma_flag_clear(self->tx_dma_periph, self->tx_dma_channel, DMA_FLAG_SDE);
	dma_flag_clear(self->tx_dma_periph, self->tx_dma_channel, DMA_FLAG_TAE);
	dma_flag_clear(self->tx_dma_periph, self->tx_dma_channel, DMA_FLAG_HTF);
	dma_flag_clear(self->tx_dma_periph, self->tx_dma_channel, DMA_FLAG_FTF);
	dma_memory_address_config( self->tx_dma_periph, self->tx_dma_channel, DMA_MEMORY_0, txd ? (uint32_t)txd : SRAM_BASE);
	dma_transfer_number_config(self->tx_dma_periph, self->tx_dma_channel, size);
	if (rxd) {
		dma_interrupt_disable(self->tx_dma_periph, self->tx_dma_channel, DMA_CHXCTL_FTFIE);
	} else {
		dma_interrupt_enable(self->tx_dma_periph, self->tx_dma_channel, DMA_CHXCTL_FTFIE);
	}
	dma_channel_enable(self->tx_dma_periph, self->tx_dma_channel);
	
	if (rxd) {
		dma_channel_disable(self->rx_dma_periph, self->rx_dma_channel);
		dma_flag_clear(self->rx_dma_periph, self->rx_dma_channel, DMA_FLAG_FEE);
		dma_flag_clear(self->rx_dma_periph, self->rx_dma_channel, DMA_FLAG_SDE);
		dma_flag_clear(self->rx_dma_periph, self->rx_dma_channel, DMA_FLAG_TAE);
		dma_flag_clear(self->rx_dma_periph, self->rx_dma_channel, DMA_FLAG_HTF);
		dma_flag_clear(self->rx_dma_periph, self->rx_dma_channel, DMA_FLAG_FTF);
		dma_memory_address_config( self->rx_dma_periph, self->rx_dma_channel, DMA_MEMORY_0, (uint32_t)rxd);
		dma_transfer_number_config(self->rx_dma_periph, self->rx_dma_channel, size);
		dma_interrupt_enable(self->rx_dma_periph, self->rx_dma_channel, DMA_CHXCTL_FTFIE);
		dma_channel_enable(self->rx_dma_periph, self->rx_dma_channel);
	}
	xEventGroupClearBits(self->event_handle, FD_SPI_EVT_FINISH);
	spi_dma_enable(self->spi_periph, SPI_DMA_TRANSMIT);
	if (rxd) {
		spi_dma_enable(self->spi_periph, SPI_DMA_RECEIVE);
	}
	EventBits_t evt_bits = xEventGroupWaitBits(self->event_handle, FD_SPI_EVT_FINISH, pdTRUE, pdFALSE, timeout);
	
	while (spi_i2s_flag_get(self->spi_periph, SPI_FLAG_TRANS)) {
		taskYIELD();
	}
	dma_interrupt_disable(self->tx_dma_periph, self->tx_dma_channel, DMA_CHXCTL_FTFIE);
	dma_channel_disable(self->tx_dma_periph, self->tx_dma_channel);
	spi_dma_disable(self->spi_periph, SPI_DMA_TRANSMIT);

	if (rxd) {
		dma_interrupt_disable(self->rx_dma_periph, self->rx_dma_channel, DMA_CHXCTL_FTFIE);
		dma_channel_disable(self->rx_dma_periph, self->rx_dma_channel);
		spi_dma_disable(self->spi_periph, SPI_DMA_RECEIVE);
	}
	if ((evt_bits & FD_SPI_EVT_FINISH) == 0) {
		return FC_ERR_TIMEOUT;
	}
	return FC_OK;
}

void fd_spi_irq(fd_spi_ptr self) {
	BaseType_t task_woken = pdFALSE;
	if( xEventGroupSetBitsFromISR(self->event_handle, FD_SPI_EVT_FINISH, &task_woken ) != pdFAIL ) {
		portYIELD_FROM_ISR( task_woken );
	}
}
