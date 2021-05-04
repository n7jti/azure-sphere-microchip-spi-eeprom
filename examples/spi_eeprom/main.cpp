#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <applibs/gpio.h>
#include <applibs/log.h>

#include <sys/random.h>
#include <assert.h>

#include "../../spi25xx.h"

int compare(uint8_t *wptr, uint8_t *rptr, size_t len, uint32_t iteration, uint32_t offset)
{
	int ret = 0;
	Log_Debug("COMPARE \n");

	if(memcmp(wptr, rptr, len)==0)
	{
		Log_Debug("GOOD! Iteration: %d\n", iteration);
	}
	else
	{
		Log_Debug("MISSMATCH ERROR! \n");
		ret = -1;
		for(size_t y = 0; y < len; y++)
		{
			if (wptr[y] != rptr[y])
			{
				Log_Debug("0x%08x -- EXP VS REA - 0x%02x vs 0x%02x \n", offset + y , wptr[y], rptr[y]);
			}
		}
	}

	return ret; 
}

int main(void)
{    
	Log_Debug("Starting uChip25xx\n");

	// const struct timespec sleep1s = { 1, 0 };

	spi25xx::SpiEeprom eeprom;
	int ret = eeprom.init(1, -1);// MT3620_ISU1_SPI, MT3620_SPI_CS_A
	if (ret < 0)
		return -1; 


	// Get a buffer full of random data
	uint8_t wbuff[256]; 
	uint8_t rbuff[256];


	uint32_t test_size = 8192;

	bool passed = true; 

	for (uint32_t x = 0; x < 512; x++)
	{
		Log_Debug("WRITE & READ 64KB iteration %d\n", x);
		for (uint32_t offset = 0; offset < test_size; offset += sizeof(wbuff))
		{
			ssize_t ret; 
			getrandom(wbuff, sizeof(wbuff), 0);
			//memset(wbuff, x & 0xFF, sizeof(wbuff));
			Log_Debug("DO WRITE @ %x \n", offset);
			
			ret = eeprom.write(offset, wbuff, sizeof(wbuff));
			assert( ret >= 0 );

			Log_Debug("DO READ @ %x \n", offset);
			ret = eeprom.read(offset, rbuff, sizeof(rbuff));
			assert( ret >= 0 );

			ret = compare(wbuff, rbuff, sizeof(wbuff), x, offset);
			if (ret < 0){
				Log_Debug("DO READ @ 0x%x \n", offset);
				ret = eeprom.read(offset, rbuff, sizeof(rbuff));
				assert( ret >= 0 );

				// compare a 2nd time. 
				ret = compare(wbuff, rbuff, sizeof(wbuff), x, offset);
				
				passed = false;

				while (true) {} // trap
			}
					
		} 
	}

	if (passed == true){
		Log_Debug("PASSED!!\n");
	}
	else{
		Log_Debug("FALIED!!\n");
	}

	while (true)
	{
		// trap!
	}

}
