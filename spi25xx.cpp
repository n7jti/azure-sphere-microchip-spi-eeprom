#include "spi25xx.h"

#include <string.h>
#include <unistd.h>

#include <applibs/log.h>

namespace spi25xx {

	constexpr uint32_t PAGE_SIZE = 32;
	constexpr uint32_t PAGE_MASK = 0x001F;

	enum Instruction : uint8_t {
		READ = 0b0011,
		WRITE = 0b0010,
		WRDI = 0b0100,
		WREN = 0b0110,
		RDSR = 0b0101,
		WRSR = 0b0001,
	};

	enum StatusBit : uint8_t {
		WIP = 0,
		WEL = 1,
		BP0 = 2,
		BP1 = 3,
	};

	SpiEeprom::SpiEeprom()
		: _spiFd(-1)
	{


	}

	int SpiEeprom::init(SPI_InterfaceId spiInterface, SPI_ChipSelectId spiChipSelect)
	{

		// Initialize the SPI master
		SPIMaster_Config config;

		int ret = SPIMaster_InitConfig(&config);
		if (ret != 0) {
			Log_Debug("ERROR: SPIMaster_InitConfig = %d errno = %s (%d)\n", ret, strerror(errno),
				errno);
			return -1;
		}
		config.csPolarity = SPI_ChipSelectPolarity_ActiveLow;
		_spiFd = SPIMaster_Open(spiInterface, spiChipSelect, &config);
		if (_spiFd < 0) {
			Log_Debug("ERROR: SPIMaster_Open: errno=%d (%s)\n", errno, strerror(errno));
			return -1;
		}

		int result = SPIMaster_SetBusSpeed(_spiFd, 400000);
		if (result != 0) {
			Log_Debug("ERROR: SPIMaster_SetBusSpeed: errno=%d (%s)\n", errno, strerror(errno));
			return -1;
		}

		result = SPIMaster_SetMode(_spiFd, SPI_Mode_0);
		if (result != 0) {
			Log_Debug("ERROR: SPIMaster_SetMode: errno=%d (%s)\n", errno, strerror(errno));
			return -1;
		}

		return 0;
	}

	void SpiEeprom::write_block(uint32_t offset, uint8_t* data, uint32_t cbSize) {
		int ret;

		uint8_t writeHeader[3];
		SPIMaster_Transfer xfer[2];
		SPIMaster_InitTransfers(xfer, 2);
		writeHeader[0] = WRITE;
		writeHeader[1] = reinterpret_cast<uint8_t*>(&offset)[1]; 
		writeHeader[2] = reinterpret_cast<uint8_t*>(&offset)[0];

		// Set the headers
		xfer[0].flags = SPI_TransferFlags_Write;
		xfer[0].readData = nullptr;
		xfer[0].length = sizeof(writeHeader);
		xfer[0].writeData = writeHeader;

		// Set the data
		xfer[1].flags = SPI_TransferFlags_Write;
		xfer[1].readData = nullptr;
		xfer[1].length = cbSize;
		xfer[1].writeData = data;

		wren();
		ret = SPIMaster_TransferSequential(_spiFd, xfer, 2);
		if (ret < 0) {
			Log_Debug("ERROR: SPIMaster_TransferSequential: errno=%d (%s)\n", errno, strerror(errno));
		}

		// Busy wait for any pending write-operation to complete
		uint8_t status;
		do {
			status = readStatus();
		} while ((status & (1 << WIP)) != 0);
	}

	void SpiEeprom::write(uint32_t offset, uint8_t *data, uint32_t size)
	{

		while (size > 0) {
			uint32_t page_offset;
			uint32_t max_page_write_size;
			uint32_t block_write_size;

			// Find the offset within the page (look at the last 5 bits)
			page_offset = offset & PAGE_MASK;

			// calculate the number of bytes left in that page;
			max_page_write_size = PAGE_SIZE - page_offset;

			// We will write the rest of the page, or the rest of the bytes, whichever is smaller. 
			block_write_size = (size > max_page_write_size) ? max_page_write_size : size;

			// Write the block to disk
			write_block(offset, data, block_write_size);

			// Update our parameters. 
			offset += block_write_size;
			data += block_write_size;
			size -= block_write_size;
		}

		return;
	}

	void SpiEeprom::read(uint32_t offset, uint8_t *data, uint32_t size)
	{
		ssize_t ret; 
		uint8_t instruction[3];
		instruction[0] = READ;
		instruction[1] = static_cast<uint8_t>(offset >> 8);
		instruction[2] = static_cast<uint8_t>(offset & 0xFF);

		ret = SPIMaster_WriteThenRead(_spiFd, instruction, sizeof(instruction), data, size);
		(void) ret; // cast ret to void to suppress teh warning that ret is never used. 
		// Log_Debug("SPIMaster_WriteThenRead: bytes=%d\n", ret);
		return; 
	}

	uint8_t SpiEeprom::readStatus()
	{
		ssize_t bytes;
		uint8_t status = 0;
		uint8_t instruction;
		instruction = RDSR;
		bytes = SPIMaster_WriteThenRead(_spiFd, &instruction, sizeof(instruction), &status, sizeof(status));
		(void) bytes; // cast bytes to void to suppress the warning that bytes are never used. 
		// Log_Debug("readStatus -- SPIMaster_WriteThenRead: bytes=%d\n", bytes);
		return status;
	}

	void SpiEeprom::writeStatus(uint8_t status)
	{
		ssize_t bytes; 
		uint8_t buffer[2];
		buffer[0] = WRSR;
		buffer[1] = status;
		bytes = ::write(_spiFd, &buffer, sizeof(buffer));
		(void) bytes; // cast bytes to void to suppress the warning that bytes is not used. 
		// Log_Debug("writeStatus -- write: bytes=%d\n", bytes);
	}

	void SpiEeprom::wren() {
		ssize_t bytes; 
		uint8_t buffer = WREN; 
		bytes = ::write(_spiFd, &buffer, sizeof(buffer));
		(void) bytes; // cast bytes to void to suppress the error that bytes is not used. 
		return; 
	}

	void SpiEeprom::wrdi() {
		ssize_t bytes; 
		uint8_t buffer = WRDI;
		bytes = ::write(_spiFd, &buffer, sizeof(buffer));
		(void) bytes; // cast bytes to void to suppress the error that bytes is not used. 
		return;
	}


} // namespace spi25xx