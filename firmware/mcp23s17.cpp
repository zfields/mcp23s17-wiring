/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */

#include "mcp23s17.h"

mcp23s17::mcp23s17 (
	const HardwareAddress hw_addr_
) :
	_hw_addr(static_cast<HardwareAddress>(0))
{
	return;
}

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
