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
	_SPI_BUS_ADDRESS(0x40 | (hw_addr_ << 1))
{
	for ( int i = 0 ; i < REGISTER_COUNT ; ++i ) {
		_control_register_address[i] = i;
	}

	return;
}

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
