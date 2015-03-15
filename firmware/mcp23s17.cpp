/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */

#include "mcp23s17.h"

#if defined(TESTING)
  #include "test/MOCK_spi.h"
#elif defined(ARDUINO) && (ARDUINO <= 100)
  #include "Arduino.h"
#elif defined(SPARK)
  #include "application.h"
#else
  #include "WProgram.h"
#endif

mcp23s17::mcp23s17 (
	const HardwareAddress hw_addr_
) :
	_SPI_BUS_ADDRESS(0x40 | (hw_addr_ << 1)),
	_control_register{ 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	_control_register_address{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }
{
	SPI.begin();
	return;
}

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
