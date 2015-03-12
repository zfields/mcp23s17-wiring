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
	_hw_addr(hw_addr_)
{
	return;
}

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
