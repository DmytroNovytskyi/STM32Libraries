/**
 * @brief STM32 HAL-based library for nRF24L01+ transceiver.
 * Configured SPI baund rate should be <8 Mbps. Recommended values are 1 Mbps or 2 Mbps.
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#ifndef NRF24L01_H
#define NRF24L01_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "stm32f1xx_hal.h"

/**
 * Defines the cache size for NRF24L01 devices. MUST be set according to number of connected devices.
 */
#define NRF24L01_DEVICES 1

#define NRF24L01_CMD_DUMMY_LOAD 0xFF
#define NRF24L01_RX_PIPE_NUMBER_EMPTY 0x07

#define NRF24L01_CMD_R_REGISTER 0x00
#define NRF24L01_CMD_W_REGISTER 0x20
#define NRF24L01_CMD_R_RX_PAYLOAD 0x61
#define NRF24L01_CMD_W_TX_PAYLOAD 0xA0
#define NRF24L01_CMD_FLUSH_TX 0xE1
#define NRF24L01_CMD_FLUSH_RX 0xE2
#define NRF24L01_CMD_REUSE_TX_PL 0xE3
#define NRF24L01_CMD_R_RX_PL_WID 0x60
#define NRF24L01_CMD_W_ACK_PAYLOAD 0xA8
#define NRF24L01_CMD_W_TX_PAYLOAD_NOACK 0xB0
#define NRF24L01_CMD_NOP 0xFF

#define NRF24L01_CMD_ACTIVATE_FEATURES 0x50
#define NRF24L01_CMD_ACTIVATE_FEATURES_KEY 0x73

#define NRF24L01_REG_CONFIG 0x00
#define NRF24L01_REG_EN_AA 0x01
#define NRF24L01_REG_EN_RXADDR 0x02
#define NRF24L01_REG_SETUP_AW 0x03
#define NRF24L01_REG_SETUP_RETR 0x04
#define NRF24L01_REG_RF_CH 0x05
#define NRF24L01_REG_RF_SETUP 0x06
#define NRF24L01_REG_STATUS 0x07
#define NRF24L01_REG_OBSERVE_TX 0x08
#define NRF24L01_REG_RPD 0x09
#define NRF24L01_REG_RX_ADDR_P0 0x0A
#define NRF24L01_REG_RX_ADDR_P1 0x0B
#define NRF24L01_REG_RX_ADDR_P2 0x0C
#define NRF24L01_REG_RX_ADDR_P3 0x0D
#define NRF24L01_REG_RX_ADDR_P4 0x0E
#define NRF24L01_REG_RX_ADDR_P5 0x0F
#define NRF24L01_REG_TX_ADDR 0x10
#define NRF24L01_REG_RX_PW_P0 0x11
#define NRF24L01_REG_RX_PW_P1 0x12
#define NRF24L01_REG_RX_PW_P2 0x13
#define NRF24L01_REG_RX_PW_P3 0x14
#define NRF24L01_REG_RX_PW_P4 0x15
#define NRF24L01_REG_RX_PW_P5 0x16
#define NRF24L01_REG_FIFO_STATUS 0x17
#define NRF24L01_REG_DYNPD 0x1C
#define NRF24L01_REG_FEATURE 0x1D

#define NRF24L01_REG_CONFIG_PRIM_RX_BIT_MASK 0x01
#define NRF24L01_REG_CONFIG_PWR_UP_BIT_MASK 0x02
#define NRF24L01_REG_STATUS_RESET_FLAGS 0x70
#define NRF24L01_REG_FEATURE_ENABLE_DYNAMIC_PAYLOAD 0x04

/**
 * @brief Address width options
 */
typedef enum {
	ADDR_WIDTH_3BYTES = 0x01,
	ADDR_WIDTH_4BYTES = 0x02,
	ADDR_WIDTH_5BYTES = 0x03
} ADDRESS_WIDTH;

/**
 * @brief Retransmit count options
 *
 * Defines the number of retransmit attempts for failed transmissions.
 * Values range from 0 (disabled) to 15 retries.
 */
typedef enum {
	RETR_DISABLED = 0x00,
	RETR_TIMES_1 = 0x01,
	RETR_TIMES_2 = 0x02,
	RETR_TIMES_3 = 0x03,
	RETR_TIMES_4 = 0x04,
	RETR_TIMES_5 = 0x05,
	RETR_TIMES_6 = 0x06,
	RETR_TIMES_7 = 0x07,
	RETR_TIMES_8 = 0x08,
	RETR_TIMES_9 = 0x09,
	RETR_TIMES_10 = 0x0A,
	RETR_TIMES_11 = 0x0B,
	RETR_TIMES_12 = 0x0C,
	RETR_TIMES_13 = 0x0D,
	RETR_TIMES_14 = 0x0E,
	RETR_TIMES_15 = 0x0F
} RETRANSMIT_COUNT;

/**
 * @brief Retransmit delay options
 *
 * Defines the delay between retransmit attempts.
 * Each value corresponds to a specific delay duration in microseconds.
 */
typedef enum {
	RETR_DELAY_250US = 0x00,
	RETR_DELAY_500US = 0x10,
	RETR_DELAY_750US = 0x20,
	RETR_DELAY_1000US = 0x30,
	RETR_DELAY_1250US = 0x40,
	RETR_DELAY_1500US = 0x50,
	RETR_DELAY_1750US = 0x60,
	RETR_DELAY_2000US = 0x70,
	RETR_DELAY_2250US = 0x80,
	RETR_DELAY_2500US = 0x90,
	RETR_DELAY_2750US = 0xA0,
	RETR_DELAY_3000US = 0xB0,
	RETR_DELAY_3250US = 0xC0,
	RETR_DELAY_3500US = 0xD0,
	RETR_DELAY_3750US = 0xE0,
	RETR_DELAY_4000US = 0xF0
} RETRANSMIT_DELAY;

/**
 * @brief RF Power levels
 */
typedef enum {
	RF_PWR_NEG18DBM = 0x00,
	RF_PWR_NEG12DBM = 0x02,
	RF_PWR_NEG6DBM = 0x04,
	RF_PWR_0DBM = 0x06
} RF_POWER;

/**
 * @brief Data rate options
 *
 * Both modules must be configured with the same data rate for proper communication.
 */
typedef enum {
	DATA_RATE_1MBPS = 0x00,
	DATA_RATE_2MBPS = 0x08,
	DATA_RATE_250KBPS = 0x20
} DATA_RATE;

typedef union {
	uint8_t value;
	struct {
		uint8_t txFifoFull :1; //TX_FULL
		uint8_t rxPipeNumber :3; //RX_P_NO
		uint8_t maxRetransmitsReached :1; //MAX_RT
		uint8_t dataSent :1; //TX_DS
		uint8_t dataReady :1; //RX_DR
		uint8_t reserved :1; //Reserved
	};
} STATUS_Register;

/**
 * @brief Configuration for single RX pipe
 *
 * Fields:
 * - index: 				   Pipe index (0–5)
 * - enable: 				   Enable this RX pipe
 * - autoAck: 				   Enable automatic acknowledgment
 * - address: 				   Address for this RX pipe
 *            				   For pipes 0 and 1: full address (matching configured address width)
 *            				   For pipes 2–5: only the least significant byte is used (must be 1 byte);
 *                             the upper bytes are inherited from pipe 1
 * - size: 					   Fixed payload size (ignored if dynamic enabled). Max value is 32.
 * 							   Should be equal to packet size for fixed length.
 * - enableDynamicPayloadSize: Enable dynamic payload length
 *
 * Notes:
 * - If dynamic payload size is enabled, 'size' is ignored.
 * - The address for pipes 2–5 must be exactly 1 byte long (i.e., only the LSB matters).
 *   They share the higher address bytes with pipe 1.
 */
typedef struct {
	uint8_t index;
	bool enable;
	bool autoAck;
	uint64_t address;
	uint8_t size;
	bool enableDynamicPayloadSize;
} NRF24L01_RxPipe;

/**
 * @brief Configuration for one RX pipe
 *
 * Fields:
 * - index: 				   Pipe index (0–5)
 * - enable: 				   Enable this RX pipe
 * - autoAck: 				   Enable automatic acknowledgment
 * - address: 				   Address for this RX pipe
 * - size: 					   Fixed payload size (ignored if dynamic enabled)
 * - enableDynamicPayloadSize: Enable dynamic payload length
 *
 * @example TX configuration:
 * NRF24L01_Config config = {
 *     .addressWidth = ADDR_WIDTH_3BYTES,
 *     .retransmitCount = RETR_TIMES_15,
 *     .retransmitDelay = RETR_DELAY_1000US,
 *     .channel = 1,
 *     .rfPower = RF_PWR_NEG6DBM,
 *     .dataRate = DATA_RATE_250KBPS,
 *     .rxPipes = {
 *         { 0, true, true, 0x112233, 0, true } //Only pipe 0 can be used to accept ACK in TX mode
 *     },
 *     .txPipeAddress = 0x112233,
 *     .enableDynamicPayloadSizeFeature = true
 * };
 *
 * @example RX configuration:
 * NRF24L01_Config config = {
 *     .addressWidth = ADDR_WIDTH_3BYTES,
 *     .channel = 1,
 *     .rfPower = RF_PWR_NEG6DBM,
 *     .dataRate = DATA_RATE_250KBPS,
 *     .rxPipes = {
 *	 	   { 1, false, false, 0x002233, 0, false }, //Configured address required for pipes 2-5
 *	 	   { 5, true, true, 0x11, 0, true } 		//Combined with pipe 1 address the result address is 0x112233
 *     },
 *     .enableDynamicPayloadSizeFeature = true
 * };
 */
typedef struct {
	ADDRESS_WIDTH addressWidth;
	RETRANSMIT_COUNT retransmitCount;
	RETRANSMIT_DELAY retransmitDelay;
	uint8_t channel;
	RF_POWER rfPower;
	DATA_RATE dataRate;
	NRF24L01_RxPipe rxPipes[6];
	uint64_t txPipeAddress;
	bool enableDynamicPayloadSizeFeature;
} NRF24L01_Config;

/**
 * @brief Main device handle
 *
 * This structure holds GPIO and SPI configuration for the NRF24L01 module,
 * as well as optional flags and statistics.
 *
 * Fields:
 * - CE_Port:       			   Chip Enable GPIO port
 * - CE_Pin:        			   Chip Enable GPIO pin
 * - CSN_Port:      			   Chip Select Not GPIO port
 * - CSN_Pin:       			   Chip Select Not GPIO pin
 * - hspi:          			   Pointer to SPI handle
 * - enableStatistics: 			   If true, transmission statistics will be collected (lost & retransmitted packets)
 * - packetsLost:   			   Number of lost packets (available if enableStatistics is true).
 *                  			   This counter is cumulative since initialization or last manual reset.
 * - packetsRetransmitted: 		   Number of retransmitted packets (available if enableStatistics is true).
 *                         		   This counter is cumulative since initialization or last manual reset.
 * - powerDownBetweenTransactions: Enables power saving mode between transactions at the cost of throughput.
 *                                 Significantly reduces power usage (TX: ~2x | RX: ~4x less)
 *                                 Max transaction rate: ~150k/sec with power saving ON, ~650k/sec with it OFF.
 */
typedef struct {
	GPIO_TypeDef *CE_Port;
	uint16_t CE_Pin;
	GPIO_TypeDef *CSN_Port;
	uint16_t CSN_Pin;
	SPI_HandleTypeDef *hspi;
	bool enableStatistics;
	uint64_t packetsLost;
	uint64_t packetsRetransmitted;
	bool powerDownBetweenTransactions;
} NRF24L01_Device;

/**
 * @brief NRF24L01 runtime instance
 *
 * Links a device with its configuration and current radio mode.
 * Used as a context and for internal caching.
 *
 * Fields:
 * - device: Pointer to NRF24L01_Device
 * - config: Pointer to NRF24L01_Config
 * - mode:   Current radio mode:
 *           0x00 = TX mode,
 *           0x01 = RX mode,
 *           0xFF = uninitialized
 */
typedef struct {
	NRF24L01_Device *device;
	NRF24L01_Config *config;
	uint8_t mode;
} NRF24L01_Instance;

/**
 * @brief Initialize the nRF24L01 module with given config
 *
 * @param device Pointer to NRF24L01_Device with GPIO and SPI settings
 * @param config Pointer to NRF24L01_Config with radio settings
 */
void NRF24L01_Init(NRF24L01_Device *device, NRF24L01_Config *config);

/**
 * @brief Enable or disable power-down mode usage between transmissions
 *
 * @param device Device handle
 * @param enable Use power down between transactions
 */
void NRF24L01_UsePowerDownMode(NRF24L01_Device *device, bool enable);

/**
 * @brief Transmit one packet
 *
 * @param device Device handle
 * @param data Pointer to data to send
 * @param size Number of bytes to send (max 32)
 * @return true if transmitted successfully, false if max retries reached)
 */
bool NRF24L01_TransmitPacket(NRF24L01_Device *device, const uint8_t *data, uint8_t size);

/**
 * @brief Receive one packet from a given RX pipe
 *
 * @param device Device handle
 * @param pipe RX pipe number (0–5)
 * @param buffer Pointer to buffer to store received data
 * @param timeout Timeout in ms
 * @return true if data received, false if timeout occurred
 */
bool NRF24L01_ReceivePacket(NRF24L01_Device *device, uint8_t pipe, uint8_t *buffer, uint32_t timeout);

/**
 * @brief Transmit data of arbitrary size by splitting it into multiple packets
 *
 * Unlike NRF24L01_TransmitPacket that sends only for single packet,
 * this method automatically splits the data into packets and sends them sequentially.
 * There is no limit on the total amount of data to be transmitted.
 * The packet size must be at least 3 bytes to accommodate transaction data.
 * If the (data size + 3 additional info) is not an exact multiple of the packet size,
 * the remaining bytes will be padded with 0x00. These padding bytes are used only for
 * alignment and are not part of the actual transmitted data.
 *
 * @param device Device handle
 * @param data Pointer to data to send
 * @param size Total number of bytes to send (no fixed limit)
 * @param packetSize Size of each packet data will be split into(min 3, max 32).
 * @return true if all packets transmitted successfully, false otherwise
 */
bool NRF24L01_Transmit(NRF24L01_Device *device, const uint8_t *data, uint8_t size, uint8_t packetSize);

/**
 * @brief Receive data of arbitrary size by reading multiple packets
 *
 * Unlike NRF24L01_ReceivePacket that receives only single packet,
 * this method receives data split into multiple packets and combines them into a single buffer.
 * There is no limit on the total amount of data to be received.
 * Ensure that the provided buffer is large enough to hold the entire data to be received.
 * Padding bytes (0x00) added during transmission are automatically removed
 * and do not count toward the final data size.
 *
 * @param device Device handle
 * @param pipe RX pipe number (0–5)
 * @param buffer Pointer to buffer to store received data
 * @param timeout Timeout in ms for the first packet to arrive
 * @return true if full data received successfully, false if timeout occurred
 */
bool NRF24L01_Receive(NRF24L01_Device *device, uint8_t pipe, uint8_t *buffer, uint32_t timeout);

#endif // NRF24L01_H
