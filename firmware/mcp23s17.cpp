/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */

#include "mcp23s17.h"

#if defined(TESTING)
  #include "test/MOCK_wiring.h"
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
	_SPI_BUS_ADDRESS(0x40 | (static_cast<uint8_t>(hw_addr_) << 1)),
	_control_register{ 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	_control_register_address{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }
{
	SPI.begin();
	return;
}

void
mcp23s17::pinMode (
	uint8_t pin_,
	PinMode mode_
) {
	uint8_t bit_mask;
	ControlRegister control_register(ControlRegister::IODIRA);
	
	digitalWrite(SS, LOW);
	SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::WRITE));
	if ( pin_ / 8 ) { control_register = ControlRegister::IODIRB; }
	SPI.transfer(static_cast<uint8_t>(control_register));
	if ( mode_ == PinMode::OUTPUT ) {
		bit_mask = ~(static_cast<uint8_t>(1) << pin_ % 8);
	} else {
		bit_mask = (static_cast<uint8_t>(1) << pin_ % 8);
	}
	SPI.transfer(bit_mask);
	digitalWrite(SS, HIGH);
	
	return;
}

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
