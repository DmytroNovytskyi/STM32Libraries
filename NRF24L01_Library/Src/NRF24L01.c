/**
 * @brief Implementation of nRF24L01+ library for STM32 using HAL
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#include "NRF24L01.h"

static NRF24L01_Instance cache[NRF24L01_DEVICES];
static uint8_t cacheSize = 0;
static uint8_t ticksPerUs = 0;

static void NRF24L01_CSNLow(NRF24L01_Device *device) {
	HAL_GPIO_WritePin(device->CSN_Port, device->CSN_Pin, GPIO_PIN_RESET);
}

static void NRF24L01_CSNHigh(NRF24L01_Device *device) {
	HAL_GPIO_WritePin(device->CSN_Port, device->CSN_Pin, GPIO_PIN_SET);
}

static void NRF24L01_CELow(NRF24L01_Device *device) {
	HAL_GPIO_WritePin(device->CE_Port, device->CE_Pin, GPIO_PIN_RESET);
}

static void NRF24L01_CEHigh(NRF24L01_Device *device) {
	HAL_GPIO_WritePin(device->CE_Port, device->CE_Pin, GPIO_PIN_SET);
}

static void NRF24L01_ReadRegister(NRF24L01_Device *device, uint8_t address, uint8_t *buffer, uint8_t size) {
	uint8_t command = NRF24L01_CMD_R_REGISTER | address;
	uint8_t *request = malloc(size + 1);
	uint8_t *response = malloc(size + 1);
	request[0] = command;
	memset(&request[1], NRF24L01_CMD_DUMMY_LOAD, size);
	memset(response, 0, size + 1);

	NRF24L01_CSNLow(device);
	HAL_SPI_TransmitReceive(device->hspi, request, response, size + 1, 50);
	NRF24L01_CSNHigh(device);

	memcpy(buffer, &response[1], size);
	free(request);
	free(response);
}

static void NRF24L01_WriteRegister(NRF24L01_Device *device, uint8_t address, const uint8_t *data, uint8_t size) {
	uint8_t command = NRF24L01_CMD_W_REGISTER | address;
	uint8_t *request = malloc(size + 1);
	request[0] = command;
	memcpy(&request[1], data, size);

	NRF24L01_CSNLow(device);
	HAL_SPI_Transmit(device->hspi, request, size + 1, 50);
	NRF24L01_CSNHigh(device);

	free(request);
}

static void NRF24L01_SendCommand(NRF24L01_Device *device, uint8_t command) {
	NRF24L01_CSNLow(device);
	HAL_SPI_Transmit(device->hspi, &command, 1, 50);
	NRF24L01_CSNHigh(device);
}

static NRF24L01_Instance* NRF24L01_GetInstanceCache(NRF24L01_Device *device) {
	for (int i = 0; i < cacheSize; i++) {
		NRF24L01_Instance *cachedInstance = &cache[i];
		if (cachedInstance->device == device) {
			return cachedInstance;
		}
	}
	return NULL;
}

static void NRF24L01_PutConfigCache(NRF24L01_Device *device, NRF24L01_Config *config) {
	for (int i = 0; i < cacheSize; i++) {
		NRF24L01_Device *cachedDevice = cache[i].device;
		if (cachedDevice == device) {
			cache[i].config = config;
			cache[i].mode = 0xFF;
			return;
		}
	}
	cache[cacheSize].device = device;
	cache[cacheSize].config = config;
	cache[cacheSize].mode = 0xFF;
	cacheSize++;
}

static void NRF24L01_InitDWT(void) {
	if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	}
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	ticksPerUs = SystemCoreClock / 1000000;
}

static void NRF24L01_DelayUs(uint32_t us) {
	uint32_t start = DWT->CYCCNT;
	uint32_t delayTicks = us * ticksPerUs;
	while ((DWT->CYCCNT - start) < delayTicks)
		;
}

static void NRF24L01_PowerUp(NRF24L01_Device *device) {
	uint8_t config;
	NRF24L01_ReadRegister(device, NRF24L01_REG_CONFIG, &config, 1);
	config |= NRF24L01_REG_CONFIG_PWR_UP_BIT_MASK;
	NRF24L01_WriteRegister(device, NRF24L01_REG_CONFIG, &config, 1);
	NRF24L01_DelayUs(4500); //Worst case scenario(External crystal, Ls = 90mH)
}

static void NRF24L01_PowerDown(NRF24L01_Device *device) {
	uint8_t config;
	NRF24L01_ReadRegister(device, NRF24L01_REG_CONFIG, &config, 1);
	config &= ~NRF24L01_REG_CONFIG_PWR_UP_BIT_MASK;
	NRF24L01_WriteRegister(device, NRF24L01_REG_CONFIG, &config, 1);
}

static void NRF24L01_ReceiveMode(NRF24L01_Device *device) {
	//Bypass already configured receive mode
	NRF24L01_Instance *instance = NRF24L01_GetInstanceCache(device);
	if (instance->mode == 0x01) {
		return;
	}

	uint8_t config;
	NRF24L01_ReadRegister(device, NRF24L01_REG_CONFIG, &config, 1);
	config |= NRF24L01_REG_CONFIG_PRIM_RX_BIT_MASK;
	NRF24L01_WriteRegister(device, NRF24L01_REG_CONFIG, &config, 1);
	instance->mode = 0x01;
	NRF24L01_DelayUs(130); //Standby modes -> TX/RX mode
}

static void NRF24L01_TransmitMode(NRF24L01_Device *device) {
	//Bypass already configured receive mode
	NRF24L01_Instance *instance = NRF24L01_GetInstanceCache(device);
	if (instance->mode == 0x00) {
		return;
	}

	uint8_t config;
	NRF24L01_ReadRegister(device, NRF24L01_REG_CONFIG, &config, 1);
	config &= ~NRF24L01_REG_CONFIG_PRIM_RX_BIT_MASK;
	NRF24L01_WriteRegister(device, NRF24L01_REG_CONFIG, &config, 1);
	instance->mode = 0x00;
	NRF24L01_DelayUs(130); //Standby modes -> TX/RX mode
}

static uint8_t NRF24L01_ResolveAddressWidth(ADDRESS_WIDTH addressWidth) {
	switch (addressWidth) {
	case ADDR_WIDTH_3BYTES:
		return 3;
	case ADDR_WIDTH_4BYTES:
		return 4;
	case ADDR_WIDTH_5BYTES:
		return 5;
	default:
		return 3;
	}
}

static void NRF24L01_ConvertAddress(uint64_t address, uint8_t *buffer, uint8_t size) {
	for (int i = 0; i < size; i++) {
		buffer[i] = (address >> (size - i - 1) * 8);
	}
}

static uint8_t NRF24L01_GetReceivedPayloadSizeForPipe(NRF24L01_Device *device, uint8_t index) {
	NRF24L01_RxPipe *cachedPipesConfig = NRF24L01_GetInstanceCache(device)->config->rxPipes;
	NRF24L01_RxPipe pipeConfig = { 0 };
	uint8_t payloadSize = 0x00;
	for (int i = 0; i < 6; ++i) {
		if (cachedPipesConfig[i].index == index) {
			pipeConfig = cachedPipesConfig[i];
			break;
		}
	}

	if (pipeConfig.enableDynamicPayloadSize) {
		uint8_t readDynamicPayloadSizeRequest[2] = { NRF24L01_CMD_R_RX_PL_WID, 0xFF };
		NRF24L01_CSNLow(device);
		HAL_SPI_TransmitReceive(device->hspi, readDynamicPayloadSizeRequest, readDynamicPayloadSizeRequest, 2, 50);
		NRF24L01_CSNHigh(device);
		payloadSize = readDynamicPayloadSizeRequest[1];
	} else {
		payloadSize = pipeConfig.size;
	}

	return payloadSize;
}

static STATUS_Register NRF24L01_GetStatus(NRF24L01_Device *device) {
	uint8_t nop = NRF24L01_CMD_NOP;
	STATUS_Register status;
	NRF24L01_CSNLow(device);
	HAL_SPI_TransmitReceive(device->hspi, &nop, &status.value, 1, 50);
	NRF24L01_CSNHigh(device);
	return status;
}

static void NRF24L01_ResetStatus(NRF24L01_Device *device) {
	uint8_t resetFlags = NRF24L01_REG_STATUS_RESET_FLAGS;
	NRF24L01_WriteRegister(device, NRF24L01_REG_STATUS, &resetFlags, 1);
}

static void NRF24L01_UpdateStatistic(NRF24L01_Device *device) {
	if (!device->enableStatistics) {
		return;
	}

	//Read OBSERVE_TX and update stats
	uint8_t data = 0x00;
	NRF24L01_ReadRegister(device, NRF24L01_REG_OBSERVE_TX, &data, 1);
	device->packetsLost += data >> 4;
	device->packetsRetransmitted += data & 0x0F;

	//Reset PLOS_CNT
	NRF24L01_Instance *instance = NRF24L01_GetInstanceCache(device);
	uint8_t rfChannel = instance->config->channel;
	NRF24L01_WriteRegister(device, NRF24L01_REG_RF_CH, &rfChannel, 1);
}

static bool NRF24L01_WaitForTransmission(NRF24L01_Device *device, uint32_t timeout) {
	bool result = false;
	STATUS_Register status;
	uint32_t start = HAL_GetTick();
	while ((HAL_GetTick() - start) < timeout) {
		status = NRF24L01_GetStatus(device);
		if (status.dataSent) {
			result = true;
			break;
		}
		if (status.maxRetransmitsReached) {
			break;
		}
	}

	NRF24L01_UpdateStatistic(device);
	NRF24L01_ResetStatus(device);
	return result;
}

void NRF24L01_Init(NRF24L01_Device *device, NRF24L01_Config *config) {
	if (config->channel > 127) {
		return;
	}
	NRF24L01_InitDWT(); //Prepare us delay functionality
	NRF24L01_PutConfigCache(device, config);

	HAL_Delay(100); //Power on reset transition state
	NRF24L01_SendCommand(device, NRF24L01_CMD_NOP); //Init SPI clock

	//Build registers values for pipes configuration
	uint8_t pipeBit = 0x00;
	uint8_t enableAutoAckPipesValue = 0x00;
	uint8_t enableRxPipesValue = 0x00;
	uint8_t enableDynamicPayloadSizeValue = 0x00;
	for (int i = 0; i < 6; i++) {
		NRF24L01_RxPipe pipe = config->rxPipes[i];
		if (pipe.enable) {
			pipeBit = 1 << pipe.index;
			enableRxPipesValue |= pipeBit;
			if (pipe.autoAck) {
				enableAutoAckPipesValue |= pipeBit;
			}
			if (pipe.enableDynamicPayloadSize) {
				enableDynamicPayloadSizeValue |= pipeBit;
			}
		}
	}
	uint8_t automaticRetransmission = config->retransmitDelay | config->retransmitCount;
	uint8_t rfSetup = config->rfPower | config->dataRate;

	//General configuration(registers 0x01 - 0x06)
	NRF24L01_WriteRegister(device, NRF24L01_REG_EN_AA, &enableAutoAckPipesValue, 1);
	NRF24L01_WriteRegister(device, NRF24L01_REG_EN_RXADDR, &enableRxPipesValue, 1);
	NRF24L01_WriteRegister(device, NRF24L01_REG_SETUP_AW, &config->addressWidth, 1);
	NRF24L01_WriteRegister(device, NRF24L01_REG_SETUP_RETR, &automaticRetransmission, 1);
	NRF24L01_WriteRegister(device, NRF24L01_REG_RF_CH, &config->channel, 1);
	NRF24L01_WriteRegister(device, NRF24L01_REG_RF_SETUP, &rfSetup, 1);

	//Configure RX pipes
	uint8_t addressWidth = NRF24L01_ResolveAddressWidth(config->addressWidth);
	uint8_t *addressBuffer = (uint8_t*) malloc(addressWidth * sizeof(uint8_t));
	NRF24L01_RxPipe pipe;
	for (int i = 0; i < 6; i++) {
		pipe = config->rxPipes[i];
		NRF24L01_ConvertAddress(pipe.address, addressBuffer, addressWidth);

		//Used for configuring address of pipes 2-5
		if (pipe.index == 1) {
			NRF24L01_WriteRegister(device, NRF24L01_REG_RX_ADDR_P1, addressBuffer, addressWidth);
		}

		if (pipe.enable) {
			switch (pipe.index) {
			case 0:
				NRF24L01_WriteRegister(device, NRF24L01_REG_RX_ADDR_P0, addressBuffer, addressWidth);
				NRF24L01_WriteRegister(device, NRF24L01_REG_RX_PW_P0, &pipe.size, 1);
				break;
			case 1:
				NRF24L01_WriteRegister(device, NRF24L01_REG_RX_PW_P1, &pipe.size, 1);
				break;
			case 2:
				NRF24L01_WriteRegister(device, NRF24L01_REG_RX_ADDR_P2, &addressBuffer[addressWidth - 1], 1);
				NRF24L01_WriteRegister(device, NRF24L01_REG_RX_PW_P2, &pipe.size, 1);
				break;
			case 3:
				NRF24L01_WriteRegister(device, NRF24L01_REG_RX_ADDR_P3, &addressBuffer[addressWidth - 1], 1);
				NRF24L01_WriteRegister(device, NRF24L01_REG_RX_PW_P3, &pipe.size, 1);
				break;
			case 4:
				NRF24L01_WriteRegister(device, NRF24L01_REG_RX_ADDR_P4, &addressBuffer[addressWidth - 1], 1);
				NRF24L01_WriteRegister(device, NRF24L01_REG_RX_PW_P4, &pipe.size, 1);
				break;
			case 5:
				NRF24L01_WriteRegister(device, NRF24L01_REG_RX_ADDR_P5, &addressBuffer[addressWidth - 1], 1);
				NRF24L01_WriteRegister(device, NRF24L01_REG_RX_PW_P5, &pipe.size, 1);
				break;
			}
		}
	}

	//Set TX address
	NRF24L01_ConvertAddress(config->txPipeAddress, addressBuffer, addressWidth);
	NRF24L01_WriteRegister(device, NRF24L01_REG_TX_ADDR, addressBuffer, addressWidth);
	free(addressBuffer);

	//Dynamic payload size configuration
	if (config->enableDynamicPayloadSizeFeature) {
		uint8_t featureRegisterValue = NRF24L01_REG_FEATURE_ENABLE_DYNAMIC_PAYLOAD;
		NRF24L01_SendCommand(device, NRF24L01_CMD_ACTIVATE_FEATURES); //Activate advanced features
		NRF24L01_SendCommand(device, NRF24L01_CMD_ACTIVATE_FEATURES_KEY);
		NRF24L01_WriteRegister(device, NRF24L01_REG_FEATURE, &featureRegisterValue, 1);
		NRF24L01_WriteRegister(device, NRF24L01_REG_DYNPD, &enableDynamicPayloadSizeValue, 1);
	}

	bool powerDownBetweenTransactions = NRF24L01_GetInstanceCache(device)->device->powerDownBetweenTransactions;
	if (!powerDownBetweenTransactions) {
		NRF24L01_PowerUp(device);
	}
}

void NRF24L01_UsePowerDownMode(NRF24L01_Device *device, bool enable) {
	if (!device->powerDownBetweenTransactions && enable) {
		NRF24L01_PowerDown(device);
	}
	if (device->powerDownBetweenTransactions && !enable) {
		NRF24L01_PowerUp(device);
	}
	device->powerDownBetweenTransactions = enable;
}

bool NRF24L01_TransmitPacket(NRF24L01_Device *device, const uint8_t *data, uint8_t size) {
	bool powerDownBetweenTransactions = NRF24L01_GetInstanceCache(device)->device->powerDownBetweenTransactions;
	bool result = false;
	if (size > 32) {
		return result;
	}

	//Prepare request
	uint8_t *request = malloc(size + 1);
	request[0] = NRF24L01_CMD_W_TX_PAYLOAD;
	memcpy(&request[1], data, size);

	if (powerDownBetweenTransactions) {
		NRF24L01_PowerUp(device);
	}

	//Write TX payload
	NRF24L01_TransmitMode(device);
	NRF24L01_SendCommand(device, NRF24L01_CMD_FLUSH_TX);
	NRF24L01_CSNLow(device);
	HAL_SPI_Transmit(device->hspi, request, size + 1, 50);
	NRF24L01_CSNHigh(device);
	free(request);

	//Transmit
	NRF24L01_CEHigh(device);
	NRF24L01_DelayUs(10);
	NRF24L01_CELow(device);

	//Wait for transmission to finish and collect statistic(if enabled)
	result = NRF24L01_WaitForTransmission(device, 100);

	if (powerDownBetweenTransactions) {
		NRF24L01_PowerDown(device);
	}
	return result;
}

bool NRF24L01_ReceivePacket(NRF24L01_Device *device, uint8_t pipe, uint8_t *buffer, uint32_t timeout) {
	bool powerDownBetweenTransactions = NRF24L01_GetInstanceCache(device)->device->powerDownBetweenTransactions;
	bool result = false;
	STATUS_Register status;
	uint8_t payloadSize;
	uint8_t *tempBuffer = NULL;

	if (powerDownBetweenTransactions) {
		NRF24L01_PowerUp(device);
	}

	//Start receiving
	NRF24L01_ReceiveMode(device);
	NRF24L01_CEHigh(device);
	uint32_t start = HAL_GetTick();

	while ((HAL_GetTick() - start) < timeout) {
		status = NRF24L01_GetStatus(device);
		if (status.dataReady) {
			NRF24L01_ResetStatus(device);
			if (status.rxPipeNumber > 5) {
				continue;
			}

			//Read RX payload
			payloadSize = NRF24L01_GetReceivedPayloadSizeForPipe(device, status.rxPipeNumber);
			tempBuffer = malloc(payloadSize + 1);
			memset(tempBuffer, 0xFF, payloadSize + 1);
			tempBuffer[0] = NRF24L01_CMD_R_RX_PAYLOAD;
			NRF24L01_CSNLow(device);
			HAL_SPI_TransmitReceive(device->hspi, tempBuffer, tempBuffer, payloadSize + 1, 50);
			NRF24L01_CSNHigh(device);

			//Copy data and finish if the pipe is correct
			if (status.rxPipeNumber == pipe) {
				memcpy(buffer, &tempBuffer[1], payloadSize);
				result = true;
				break;
			}
		}
	}

	//Finish receiving and clear RX FIFO
	free(tempBuffer);
	NRF24L01_CELow(device);
	NRF24L01_SendCommand(device, NRF24L01_CMD_FLUSH_RX);

	if (powerDownBetweenTransactions) {
		NRF24L01_PowerDown(device);
	}

	return result;
}

bool NRF24L01_Transmit(NRF24L01_Device *device, const uint8_t *data, uint8_t size, uint8_t packetSize) {
	bool result = true;
	bool powerDownBetweenTransactions = NRF24L01_GetInstanceCache(device)->device->powerDownBetweenTransactions;

	//Build message
	//3 additional bytes for first packet:
	//identifier(0x00), number of packets and packet size for receiver to expect
	uint8_t messageSize = size + 3;
	uint8_t numberOfPackets = (uint8_t) ceil((float) messageSize / packetSize);
	uint8_t fillBytesSize = packetSize - messageSize % packetSize;
	uint8_t *message = malloc(messageSize + fillBytesSize);
	message[0] = 0x00;
	message[1] = numberOfPackets;
	message[2] = packetSize;
	memcpy(&message[3], data, size);
	memset(&message[messageSize], 0x00, fillBytesSize);

	//Disable power down mode if is used
	if (powerDownBetweenTransactions) {
		NRF24L01_UsePowerDownMode(device, false);
	}

	//Send packets
	for (int i = 0; i < numberOfPackets; i++) {
		if (!NRF24L01_TransmitPacket(device, &message[i * packetSize], packetSize)) {
			result = false;
			break;
		}
	}

	//Re-enable power down mode if was used
	if (powerDownBetweenTransactions) {
		NRF24L01_UsePowerDownMode(device, true);
	}

	free(message);
	return result;
}

bool NRF24L01_Receive(NRF24L01_Device *device, uint8_t pipe, uint8_t *buffer, uint32_t timeout) {
	bool result = false;
	bool powerDownBetweenTransactions = NRF24L01_GetInstanceCache(device)->device->powerDownBetweenTransactions;
	uint8_t firstPacket[32] = { 0 };
	uint8_t numberOfPackets;
	uint8_t packetSize;
	uint8_t bytesLeft;
	uint8_t *restOfPackets;

	//Disable power down mode if is being used
	if (powerDownBetweenTransactions) {
		NRF24L01_UsePowerDownMode(device, false);
	}

	//Receive packets
	uint32_t start = HAL_GetTick();
	while ((HAL_GetTick() - start) < timeout && !result) {
		if (NRF24L01_ReceivePacket(device, pipe, firstPacket, 50)) {
			//Skip if no first packet identifier found
			if(firstPacket[0] != 0x00){
				continue;
			}
			result = true;

			//Extract transaction data
			numberOfPackets = firstPacket[1];
			packetSize = firstPacket[2];

			//Receive other packets
			bytesLeft = (numberOfPackets - 1) * packetSize;
			restOfPackets = malloc(bytesLeft);
			for (int i = 0; i < numberOfPackets - 1; i++) {
				if (!NRF24L01_ReceivePacket(device, pipe, &restOfPackets[i * packetSize], 50)) {
					result = false;
					break;
				}
			}

			//Copy data to result buffer
			if (result) {
				memcpy(buffer, &firstPacket[3], packetSize - 3);
				memcpy(&buffer[packetSize - 3], restOfPackets, bytesLeft);
			}

			free(restOfPackets);
		}
	}

	//Re-enable power down mode if was used
	if (powerDownBetweenTransactions) {
		NRF24L01_UsePowerDownMode(device, true);
	}

	return result;
}
