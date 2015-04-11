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
    //TODO: Set IOCON:HAEN bit
    //TODO: Load cache from chip registers
    return;
}

mcp23s17::PinLatchValue
mcp23s17::digitalRead (
    const uint8_t pin_
) const {
    const unsigned int bit_pos(pin_ % 8);
    
    ControlRegister latch_register(ControlRegister::GPIOA);
    ControlRegister direction_register(ControlRegister::IODIRA);
    uint8_t port_latch_values(0x00);
    
    // Select the appropriate port
    if ( pin_ / 8 ) {
        latch_register = ControlRegister::GPIOB;
        direction_register = ControlRegister::IODIRB;
    }
    
    // Check to see if device is in the proper state
    if ( PinMode::OUTPUT == static_cast<PinMode>((_control_register[static_cast<uint8_t>(direction_register)] >> bit_pos) & 0x01) ) { return PinLatchValue::LOW; }
    
    // Send data
    ::digitalWrite(SS, LOW);
    SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::READ));
    SPI.transfer(static_cast<uint8_t>(latch_register));
    port_latch_values = SPI.transfer(static_cast<uint8_t>(latch_register));  // Arbitrary bit to flush result buffer. `latch_register` is selected, because it is guaranteed to be in active memory.
    ::digitalWrite(SS, HIGH);
    
    return static_cast<PinLatchValue>((port_latch_values >> bit_pos) & 0x01);
}

void
mcp23s17::digitalWrite (
    const uint8_t pin_,
    const PinLatchValue value_
) {
    const unsigned int bit_pos(pin_ % 8);
    const uint8_t bit_mask(static_cast<uint8_t>(1) << bit_pos);
    
    ControlRegister latch_register(ControlRegister::GPIOA);
    ControlRegister direction_register(ControlRegister::IODIRA);
    uint8_t registry_value;
    
    // Select the appropriate port
    if ( pin_ / 8 ) {
        latch_register = ControlRegister::GPIOB;
        direction_register = ControlRegister::IODIRB;
    }
    
    // Check to see if device is in the proper state
    if ( PinMode::INPUT == static_cast<PinMode>((_control_register[static_cast<uint8_t>(direction_register)] >> bit_pos) & 0x01) ) { return; }
    
    // Check cache for exisiting data
    registry_value = _control_register[static_cast<uint8_t>(latch_register)];
    if ( PinLatchValue::LOW == value_ ) {
        registry_value &= ~bit_mask;
    } else {
        registry_value |= bit_mask;
    }
    
    // Test to see if bit is already set
    if ( _control_register[static_cast<uint8_t>(latch_register)] == registry_value ) { return; }
    _control_register[static_cast<uint8_t>(latch_register)] = registry_value;
    
    // Send data
    ::digitalWrite(SS, LOW);
    SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::WRITE));
    SPI.transfer(static_cast<uint8_t>(latch_register));
    SPI.transfer(registry_value);
    ::digitalWrite(SS, HIGH);
    
    return;
}

void
mcp23s17::pinMode (
    const uint8_t pin_,
    const PinMode mode_
) {
    const unsigned int bit_pos(pin_ % 8);
    const uint8_t bit_mask(static_cast<uint8_t>(1) << bit_pos);
    
    ControlRegister latch_register(ControlRegister::IODIRA);
    uint8_t registry_value;
    
    // Select the appropriate port
    if ( pin_ / 8 ) { latch_register = ControlRegister::IODIRB; }
    
    // Check cache for exisiting data
    registry_value = _control_register[static_cast<uint8_t>(latch_register)];
    switch ( mode_ ) {
      case PinMode::OUTPUT:
        registry_value &= ~bit_mask;
        break;
      case PinMode::INPUT:
      case PinMode::INPUT_PULLUP:
        registry_value |= bit_mask;
        break;
    }
    
    // Test to see if bit is already set
    if ( _control_register[static_cast<uint8_t>(latch_register)] == registry_value ) { return; }
    _control_register[static_cast<uint8_t>(latch_register)] = registry_value;
    
    // Send data
    ::digitalWrite(SS, LOW);
    SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::WRITE));
    SPI.transfer(static_cast<uint8_t>(latch_register));
    SPI.transfer(registry_value);
    ::digitalWrite(SS, HIGH);
    
    //TODO: Set GPPU[A|B] registers on mcp23s17::PinMode::INPUT_PULLUP
    return;
}

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
