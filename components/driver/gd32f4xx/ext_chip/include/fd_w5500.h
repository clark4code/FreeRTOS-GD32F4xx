#pragma once
#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum fd_w5_common_reg_t {
	FD_W5_CREG_MODE 		= 0x0000,
	FD_W5_CREG_GAR 		= 0x0001,
	FD_W5_CREG_SUBR 		= 0x0005,
	FD_W5_CREG_SHAR 		= 0x0009,
	FD_W5_CREG_SIPR 		= 0x000F,
	FD_W5_CREG_INTLEVEL	= 0x0013,
	FD_W5_CREG_IR 			= 0x0015,
	FD_W5_CREG_IMR 		= 0x0016,
	FD_W5_CREG_SIR 		= 0x0017,
	FD_W5_CREG_SIMR 		= 0x0018,
	FD_W5_CREG_RTR 		= 0x0019,
	FD_W5_CREG_RCR 		= 0x001B,
	FD_W5_CREG_PTIMER 		= 0x001C,
	FD_W5_CREG_PMAGIC 		= 0x001D,
	FD_W5_CREG_PHAR 		= 0x001E,
	FD_W5_CREG_PSID 		= 0x0024,
	FD_W5_CREG_PMRU 		= 0x0026,
	FD_W5_CREG_UIPR 		= 0x0028,
	FD_W5_CREG_UPORTR 		= 0x002C,
	FD_W5_CREG_PHYCFGR 	= 0x002E,
	FD_W5_CREG_VERSION 	= 0x0039
} fd_w5_common_reg_t;

typedef enum fd_w5_socket_reg_t {
	FD_W5_SREG_MR 			= 0x0000,
	FD_W5_SREG_CR 			= 0x0001,
	FD_W5_SREG_IR 			= 0x0002,
	FD_W5_SREG_SR 			= 0x0003,
	FD_W5_SREG_PORT 		= 0x0004,
	FD_W5_SREG_DHAR 		= 0x0006,
	FD_W5_SREG_DIPR 		= 0x000C,
	FD_W5_SREG_DPORTR		= 0x0010,
	FD_W5_SREG_MSSR		= 0x0012,
	FD_W5_SREG_TOS			= 0x0015,
	FD_W5_SREG_TTL			= 0x0016,
	FD_W5_SREG_RXBUF_SIZE	= 0x001E,
	FD_W5_SREG_TXBUF_SIZE	= 0x001F,
	FD_W5_SREG_TX_FSR		= 0x0020,
	FD_W5_SREG_TX_RD		= 0x0022,
	FD_W5_SREG_TX_WR		= 0x0024,
	FD_W5_SREG_RX_RSR		= 0x0026,
	FD_W5_SREG_RX_RD		= 0x0028,
	FD_W5_SREG_RX_WR		= 0x002A,
	FD_W5_SREG_IMR			= 0x002C,
	FD_W5_SREG_FRAG		= 0x002D,
	FD_W5_SREG_KPALVTR		= 0x002F
} fd_w5_socket_reg_t;

typedef enum fd_w5_block_select_t {
	FD_W5_BSB_COMMON_REG,
	FD_W5_BSB_SOCKET0_REG,
	FD_W5_BSB_SOCKET0_TXD,
	FD_W5_BSB_SOCKET0_RXD,
	FD_W5_BSB_SOCKET1_REG = FD_W5_BSB_SOCKET0_RXD + 2,
	FD_W5_BSB_SOCKET1_TXD,
	FD_W5_BSB_SOCKET1_RXD,
	FD_W5_BSB_SOCKET2_REG = FD_W5_BSB_SOCKET1_RXD + 2,
	FD_W5_BSB_SOCKET2_TXD,
	FD_W5_BSB_SOCKET2_RXD,
	FD_W5_BSB_SOCKET3_REG = FD_W5_BSB_SOCKET2_RXD + 2,
	FD_W5_BSB_SOCKET3_TXD,
	FD_W5_BSB_SOCKET3_RXD,
	FD_W5_BSB_SOCKET4_REG = FD_W5_BSB_SOCKET3_RXD + 2,
	FD_W5_BSB_SOCKET4_TXD,
	FD_W5_BSB_SOCKET4_RXD,
	FD_W5_BSB_SOCKET5_REG = FD_W5_BSB_SOCKET4_RXD + 2,
	FD_W5_BSB_SOCKET5_TXD,
	FD_W5_BSB_SOCKET5_RXD,
	FD_W5_BSB_SOCKET6_REG = FD_W5_BSB_SOCKET5_RXD + 2,
	FD_W5_BSB_SOCKET6_TXD,
	FD_W5_BSB_SOCKET6_RXD,
	FD_W5_BSB_SOCKET7_REG = FD_W5_BSB_SOCKET6_RXD + 2,
	FD_W5_BSB_SOCKET7_TXD,
	FD_W5_BSB_SOCKET7_RXD
} fd_w5_block_select_t;

typedef enum fd_w5_socket_config_t {
	FD_W5_SC_OPEN		= 0x01,
	FD_W5_SC_LISTEN	= 0x02,
	FD_W5_SC_CONNECT	= 0x04,
	FD_W5_SC_DISCON	= 0x08,
	FD_W5_SC_CLOSE		= 0x10,
	FD_W5_SC_SEND		= 0x20,
	FD_W5_SC_SEND_MAC	= 0x21,
	FD_W5_SC_SEND_KEEP	= 0x22,
	FD_W5_SC_RECV		= 0x40
} fd_w5_socket_config_t;

typedef struct fd_w5_param_t {
	uint8_t		gateway[4];
	uint8_t		subnet_mask[4];
	uint8_t		mac[6];
	uint8_t		source_ip[4];
	uint16_t	tcp_port;
	uint16_t	udp_port;
} fd_w5_param_t, *fd_w5_param_ptr;

typedef struct fd_w5_io_t {
	void (*set_reset)(uint8_t level);
	void (*set_spi_ss)(uint8_t level);
	int (*spi_tr)(const void *txd, void *rxd, size_t size, TickType_t timeout);
} fd_w5_io_t, *fd_w5_io_ptr;

typedef struct fd_w5_t {
	fd_w5_param_ptr		param;
	fd_w5_io_ptr			io;
	EventGroupHandle_t	event_handle;
	SemaphoreHandle_t 	mutex;
} fd_w5_t, *fd_w5_ptr;

#define FD_W5_BIT(n) (1 << (n))

typedef enum fd_w5_event_t {
	FD_W5_EVT_INT				= FD_W5_BIT(0),
	FD_W5_EVT_RESET			= FD_W5_BIT(1),
	FD_W5_EVT_TCP_CONNECTED	= FD_W5_BIT(2),
	FD_W5_EVT_TCP_TX_FINISH	= FD_W5_BIT(3),
	FD_W5_EVT_TCP_RX_READY		= FD_W5_BIT(4),
	FD_W5_EVT_TCP_TIMEOUT		= FD_W5_BIT(5),
	FD_W5_EVT_UDP_TX_FINISH	= FD_W5_BIT(6),
	FD_W5_EVT_UDP_RX_READY		= FD_W5_BIT(7)
} fd_w5_event_t, *fd_w5_event_ptr;

void fd_w5_init(fd_w5_ptr self, fd_w5_param_ptr param, fd_w5_io_ptr io);
void fd_w5_update_param(fd_w5_ptr self);
bool fd_w5_tcp_is_connected(fd_w5_ptr self);
bool fd_w5_tcp_tx(fd_w5_ptr self, const void *data, size_t size);
bool fd_w5_udp_tx(fd_w5_ptr self, const uint8_t *dest_ip, uint16_t dest_port, const void *data, size_t size);
size_t fd_w5_tcp_read(fd_w5_ptr self, void *data, size_t size);
size_t fd_w5_udp_read(fd_w5_ptr self, void *data, size_t size);
void fd_w5_int_irq(fd_w5_ptr self);