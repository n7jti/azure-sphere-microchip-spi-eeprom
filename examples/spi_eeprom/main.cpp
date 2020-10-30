#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <applibs/gpio.h>
#include <applibs/log.h>

#include "../../spi25xx.h"

int main(void)
{    
	Log_Debug("Starting uChip25xx\n");

	const struct timespec sleep1s = { 1, 0 };

	spi25xx::SpiEeprom eeprom;
	int ret = eeprom.init(1, -1);// MT3620_ISU1_SPI, MT3620_SPI_CS_A
	if (ret < 0)
		return -1; 


	// write the values 0-255 in the first 256 bytes of the EEPROM
	uint8_t buff[256]; 
	constexpr uint32_t offset = 0; 
	for (unsigned int i = 0; i < sizeof(buff); i++) {
		buff[i] = static_cast<uint8_t>(i);
		//buff[i] = 0; 
	}

	eeprom.write(offset, buff, sizeof(buff));
	memset(buff, 0, sizeof(buff));

	while (true) {
		memset(buff, 0, sizeof(buff));
		eeprom.read(offset, buff, sizeof(buff));

		for (int i = 0; i < 16; i++) {
			Log_Debug("%02x:  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				i * 16,
				buff[i * 16 + 0],
				buff[i * 16 + 1],
				buff[i * 16 + 2],
				buff[i * 16 + 3],
				buff[i * 16 + 4],
				buff[i * 16 + 5],
				buff[i * 16 + 6],
				buff[i * 16 + 7],
				buff[i * 16 + 8],
				buff[i * 16 + 9],
				buff[i * 16 + 10],
				buff[i * 16 + 11],
				buff[i * 16 + 12],
				buff[i * 16 + 13],
				buff[i * 16 + 14],
				buff[i * 16 + 15]
			);
		}

		nanosleep(&sleep1s, nullptr);
	}
	 

}
