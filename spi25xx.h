#pragma once
#include <stdint.h>
#include <errno.h>

#define SPI_STRUCTS_VERSION 1
#include <applibs/spi.h>

namespace spi25xx {

class SpiEeprom {
public: 
	SpiEeprom();
	int init(SPI_InterfaceId spiInterface, SPI_ChipSelectId spiChipSelect); 
	void write(uint32_t offset, uint8_t *data, uint32_t cbSize);
	void read(uint32_t offset, uint8_t *data, uint32_t size); 
	uint8_t readStatus();
	void writeStatus(uint8_t status);

private:
	void wren(); 
	void wrdi(); 
	void write_block(uint32_t offset, uint8_t* data, uint32_t cbSize);

	int _spiFd; 

};



} // namespace