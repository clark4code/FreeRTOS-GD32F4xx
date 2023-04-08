#include <fd_w5500.h>
#include <string.h>

void fd_w5_task(fd_w5_ptr self);
int fd_w5_tr(fd_w5_ptr self, uint16_t address, uint8_t bsb, bool is_read, void *buffer, size_t size);

void fd_w5_init(fd_w5_ptr self, fd_w5_param_ptr param, fd_w5_io_ptr io) {
	self->io = io;
	self->param = param;
	self->event_handle = xEventGroupCreate();
	self->mutex = xSemaphoreCreateMutex();
	xTaskCreate(fd_w5_task, "fd_w5_task", 128, self, configMAX_PRIORITIES - 1, NULL);
}

void fd_w5_task(fd_w5_ptr self) {
	int err;
	uint8_t buffer[20];
	uint8_t ir, sir;
	EventBits_t evt_bits;

	self->io->set_spi_ss(1);
	self->io->set_reset(0);
	vTaskDelay(pdMS_TO_TICKS(2));
	self->io->set_reset(1);
	vTaskDelay(pdMS_TO_TICKS(200));

	while (true) {
		buffer[0] = 0x80;
		err = fd_w5_tr(self, FD_W5_CREG_MODE, FD_W5_BSB_COMMON_REG, false, buffer, 1);	// 复位清空寄存器
		err = fd_w5_tr(self, FD_W5_CREG_GAR, FD_W5_BSB_COMMON_REG, false, self->param, 18);
		err = fd_w5_tr(self, FD_W5_CREG_GAR, FD_W5_BSB_COMMON_REG, true, buffer, 18);
		buffer[0] = 0x03;	// 使能socket 0, 1的中断
		err = fd_w5_tr(self, FD_W5_CREG_SIMR, FD_W5_BSB_COMMON_REG, false, buffer, 1);

		buffer[0] = (self->param->tcp_port >> 8) & 0xFF;
		buffer[1] = self->param->tcp_port & 0xFF;
		err = fd_w5_tr(self, FD_W5_SREG_PORT, FD_W5_BSB_SOCKET0_REG, false, buffer, 2);	// TCP服务端口设置
		buffer[0] = 0x01;	// 模式，0x0000	TCP
		buffer[1] = FD_W5_SC_OPEN;
		err = fd_w5_tr(self, FD_W5_SREG_MR, FD_W5_BSB_SOCKET0_REG, false, buffer, 2);
		buffer[0] = FD_W5_SC_LISTEN;
		err = fd_w5_tr(self, FD_W5_SREG_CR, FD_W5_BSB_SOCKET0_REG, false, buffer, 1);

		buffer[0] = (self->param->udp_port >> 8) & 0xFF;
		buffer[1] = self->param->udp_port & 0xFF;
		err = fd_w5_tr(self, FD_W5_SREG_PORT, FD_W5_BSB_SOCKET1_REG, false, buffer, 2);	// UDP本地端口设置
		err = fd_w5_tr(self, FD_W5_SREG_PORT, FD_W5_BSB_SOCKET1_REG, true, buffer, 2);
		buffer[0] = 0x02;	// 模式，0x0000	UDP
		buffer[1] = FD_W5_SC_OPEN;
		err = fd_w5_tr(self, FD_W5_SREG_MR, FD_W5_BSB_SOCKET1_REG, false, buffer, 2);
		err = fd_w5_tr(self, FD_W5_SREG_MR, FD_W5_BSB_SOCKET1_REG, true, buffer, 2);
		buffer[0] = FD_W5_SC_RECV;
		err = fd_w5_tr(self, FD_W5_SREG_CR, FD_W5_BSB_SOCKET1_REG, false, buffer, 1);

		
		while (true) {
			err = fd_w5_tr(self, FD_W5_CREG_SIR, FD_W5_BSB_COMMON_REG, true, buffer, 1);	// 读全局中断状态
			sir = buffer[0];
			if (sir == 0
				|| sir == 0xff) {
				evt_bits = xEventGroupWaitBits(self->event_handle, FD_W5_EVT_INT | FD_W5_EVT_RESET, pdTRUE, pdFALSE, portMAX_DELAY);
				if (evt_bits & FD_W5_EVT_RESET) {
					break;
				}
				fd_w5_tr(self, FD_W5_CREG_SIR, FD_W5_BSB_COMMON_REG, true, buffer, 1);
				sir = buffer[0];
			}
			if (sir & 0x01) {	// TCP
				err = fd_w5_tr(self, FD_W5_SREG_IR, FD_W5_BSB_SOCKET0_REG, true, buffer, 1);
				ir = buffer[0];
				if (ir & (0x1 << 0)) {			// 建立连接
					buffer[0] = FD_W5_SC_RECV;			// 开始读数据
					err = fd_w5_tr(self, FD_W5_SREG_CR, FD_W5_BSB_SOCKET0_REG, false, buffer, 1);
					xEventGroupSetBits(self->event_handle, FD_W5_EVT_TCP_CONNECTED);
				}
				if (ir & (0x1 << 1)) {	// 断开连接
					xEventGroupClearBits(self->event_handle, FD_W5_EVT_TCP_CONNECTED);
					break;
				}
				if (ir & (0x1 << 2)) {	// 收到数据
					xEventGroupSetBits(self->event_handle, FD_W5_EVT_TCP_RX_READY);
				}
				if (ir & (0x1 << 3)) {	// 超时
					xEventGroupSetBits(self->event_handle, FD_W5_EVT_TCP_TIMEOUT);
					break;
				}
				if (ir & (0x1 << 4)) {	// 发送完成
					xEventGroupSetBits(self->event_handle, FD_W5_EVT_TCP_TX_FINISH);
				}
				
				buffer[0] = 0xff;
				err = fd_w5_tr(self, FD_W5_SREG_IR, FD_W5_BSB_SOCKET0_REG, false, buffer, 1);
				
			}
			if (sir & 0x02) {	// UDP
				err = fd_w5_tr(self, FD_W5_SREG_IR, FD_W5_BSB_SOCKET1_REG, true, buffer, 1);
				ir = buffer[0];
				if (ir & (0x1 << 2)) {	// 收到数据
					xEventGroupSetBits(self->event_handle, FD_W5_EVT_UDP_RX_READY);
				}
				if (ir & (0x1 << 4)) {	// 发送完成
					xEventGroupSetBits(self->event_handle, FD_W5_EVT_UDP_TX_FINISH);
				}
				buffer[0] = 0xff;
				err = fd_w5_tr(self, FD_W5_SREG_IR, FD_W5_BSB_SOCKET1_REG, false, buffer, 1);
			}
		} 
	}
}

int fd_w5_tr(fd_w5_ptr self, uint16_t address, uint8_t bsb, bool is_read, void *buffer, size_t size) {
	uint8_t head[3] = {(address >> 8) & 0xff, address & 0xff, (bsb << 3) | (is_read ? 0u : 0x4u)};
	xSemaphoreTake(self->mutex, portMAX_DELAY);
	self->io->set_spi_ss(0);
	TickType_t timeout = pdMS_TO_TICKS(500);
	int err = self->io->spi_tr(head, NULL, 3, timeout);
	if (err != 0) {
		self->io->set_spi_ss(1);
		vTaskDelay(pdMS_TO_TICKS(1));
		xSemaphoreGive(self->mutex);
		return err;
	}
	err = self->io->spi_tr(is_read ? NULL : buffer, is_read ? buffer : NULL, size, timeout);
	self->io->set_spi_ss(1);
	vTaskDelay(pdMS_TO_TICKS(1));
	xSemaphoreGive(self->mutex);
	return err;
}

void fd_w5_update_param(fd_w5_ptr self) {
	xEventGroupSetBits(self->event_handle, FD_W5_EVT_RESET);
}

bool fd_w5_tcp_is_connected(fd_w5_ptr self) {
	return (xEventGroupWaitBits(self->event_handle, FD_W5_EVT_TCP_CONNECTED, pdFALSE, pdFALSE, 0) & FD_W5_EVT_TCP_CONNECTED) ? true : false;
}

bool fd_w5_tx(fd_w5_ptr self, uint8_t socket_no, const void *data, size_t size) {
	uint8_t buffer[6];
	uint8_t bsb_socket_reg = socket_no == 0 ? FD_W5_BSB_SOCKET0_REG : FD_W5_BSB_SOCKET1_REG,
			bsb_socket_txd = socket_no == 0 ? FD_W5_BSB_SOCKET0_TXD : FD_W5_BSB_SOCKET1_TXD;
	fd_w5_tr(self, FD_W5_SREG_TX_FSR, bsb_socket_reg, true, buffer, 6);
	uint16_t tx_free_size = (((uint16_t)buffer[0]) << 8) | buffer[1];
	if (tx_free_size < size) {
		return false;
	}
	uint16_t tx_write_pointer = (((uint16_t)buffer[4]) << 8) | buffer[5];
	fd_w5_tr(self, tx_write_pointer, bsb_socket_txd, false, data, size);

	tx_write_pointer += size;
	buffer[0] = (tx_write_pointer >> 8) & 0xff;
	buffer[1] = tx_write_pointer & 0xff;
	fd_w5_tr(self, FD_W5_SREG_TX_WR, bsb_socket_reg, false, buffer, 2);
	buffer[0] = FD_W5_SC_SEND;
	fd_w5_tr(self, FD_W5_SREG_CR, bsb_socket_reg, false, buffer, 1);
	return true;
}

bool fd_w5_tcp_tx(fd_w5_ptr self, const void *data, size_t size) {
	return fd_w5_tx(self, 0, data, size);
}

bool fd_w5_udp_tx(fd_w5_ptr self, const uint8_t *dest_ip, uint16_t dest_port, const void *data, size_t size) {
	uint8_t buffer[6];
	memcpy(buffer, dest_ip, 4);
	buffer[4] = (dest_port >> 8) & 0xFF;
	buffer[5] = dest_port & 0xFF;
	fd_w5_tr(self, FD_W5_SREG_DIPR, FD_W5_BSB_SOCKET1_REG, false, buffer, 6);
	return fd_w5_tx(self, 1, data, size);
}

size_t fd_w5_read(fd_w5_ptr self, uint8_t socket_no, void *data, size_t size) {
	uint8_t buffer[4];
	uint8_t bsb_socket_reg = socket_no == 0 ? FD_W5_BSB_SOCKET0_REG : FD_W5_BSB_SOCKET1_REG,
			bsb_socket_rxd = socket_no == 0 ? FD_W5_BSB_SOCKET0_RXD : FD_W5_BSB_SOCKET1_RXD;
	fd_w5_tr(self, FD_W5_SREG_RX_RSR, bsb_socket_reg, true, buffer, 4);
	uint16_t rx_received_size = (((uint16_t)buffer[0]) << 8) | buffer[1],
			rx_read_pointer = (((uint16_t)buffer[2]) << 8) | buffer[3];
	if (data == NULL) {
		return rx_received_size;
	}
	if (size > rx_received_size) {
		size = rx_received_size;
	}
	fd_w5_tr(self, rx_read_pointer, bsb_socket_rxd, true, data, size);
	rx_read_pointer += size;
	buffer[0] = (rx_read_pointer >> 8) & 0xFF;
	buffer[1] = rx_read_pointer & 0xFF;
	fd_w5_tr(self, FD_W5_SREG_RX_RD, bsb_socket_reg, false, buffer, 2);
	buffer[0] = FD_W5_SC_RECV;
	fd_w5_tr(self, FD_W5_SREG_CR, bsb_socket_reg, false, buffer, 1);
	return size;
}

size_t fd_w5_tcp_read(fd_w5_ptr self, void *data, size_t size) {
	return fd_w5_read(self, 0, data, size);
}

size_t fd_w5_udp_read(fd_w5_ptr self, void *data, size_t size) {
	return fd_w5_read(self, 1, data, size);
}

void fd_w5_int_irq(fd_w5_ptr self) {
	BaseType_t task_woken = pdFALSE;
	if( xEventGroupSetBitsFromISR(self->event_handle, FD_W5_EVT_INT, &task_woken ) != pdFAIL ) {
		portYIELD_FROM_ISR( task_woken );
	}
}