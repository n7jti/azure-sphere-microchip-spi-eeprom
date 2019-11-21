#pragma once
#include <stdint.h>

namespace spi25xx {

class SpiEeprom {
public: 
	SpiEeprom();
	int init(); 
	void write(uint16_t offset, uint8_t *data, uint16_t cbSize);
	void read(uint16_t offset, uint8_t *data, uint16_t size); 
	uint8_t readStatus();
	void writeStatus(uint8_t status);

private:
	void wren(); 
	void wrdi(); 
	void write_block(uint16_t offset, uint8_t* data, uint16_t cbSize);

	int _spiFd; 

};



} // namespace