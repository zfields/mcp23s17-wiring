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
    ControlRegister latch_register(ControlRegister::GPIOA);
    if ( pin_ / 8 ) {
        latch_register = ControlRegister::GPIOB;
    } else {
        if ( PinMode::OUTPUT == static_cast<PinMode>((_control_register[static_cast<uint8_t>(ControlRegister::IODIRA)] >> pin_) & 0x01) ) { return PinLatchValue::HIGH; }
    }
    
    // Send data
    ::digitalWrite(SS, LOW);
    SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::READ));
    SPI.transfer(static_cast<uint8_t>(latch_register));
    SPI.transfer(1 << (pin_ % 8));
    ::digitalWrite(SS, HIGH);
    
    return PinLatchValue::HIGH;
}

void
mcp23s17::digitalWrite (
    const uint8_t pin_,
    const PinLatchValue value_
) {
    uint8_t bit_mask;
    ControlRegister latch_register(ControlRegister::GPIOA);
    ControlRegister direction_register(ControlRegister::IODIRA);
    if ( pin_ / 8 ) {
        latch_register = ControlRegister::GPIOB;
        direction_register = ControlRegister::IODIRB;
    }

    // Check to see if device is in the proper state
    if ( ((_control_register[static_cast<uint8_t>(direction_register)] >> (pin_ % 8)) & 0x01) == static_cast<uint8_t>(PinMode::INPUT) ) { return; }
    
    // Check cache for exisiting data
    bit_mask = _control_register[static_cast<uint8_t>(latch_register)];
    if ( PinLatchValue::LOW == value_ ) {
        bit_mask &= ~(static_cast<uint8_t>(1) << pin_ % 8);
    } else {
        bit_mask |= (static_cast<uint8_t>(1) << pin_ % 8);
    }
    
    // Test to see if bit is already set
    if ( _control_register[static_cast<uint8_t>(latch_register)] == bit_mask ) { return; }
    _control_register[static_cast<uint8_t>(latch_register)] = bit_mask;
    
    // Send data
    ::digitalWrite(SS, LOW);
    SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::WRITE));
    SPI.transfer(static_cast<uint8_t>(latch_register));
    SPI.transfer(bit_mask);
    ::digitalWrite(SS, HIGH);
    
    return;
}

void
mcp23s17::pinMode (
    const uint8_t pin_,
    const PinMode mode_
) {
    uint8_t bit_mask;
    ControlRegister latch_register(ControlRegister::IODIRA);
    if ( pin_ / 8 ) { latch_register = ControlRegister::IODIRB; }
    
    // Check cache for exisiting data
    bit_mask = _control_register[static_cast<uint8_t>(latch_register)];
    if ( PinMode::INPUT == mode_ ) {
        bit_mask |= (static_cast<uint8_t>(1) << pin_ % 8);
    } else {
        bit_mask &= ~(static_cast<uint8_t>(1) << pin_ % 8);
    }
    
    // Test to see if bit is already set
    if ( _control_register[static_cast<uint8_t>(latch_register)] == bit_mask ) { return; }
    _control_register[static_cast<uint8_t>(latch_register)] = bit_mask;
    
    // Send data
    ::digitalWrite(SS, LOW);
    SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::WRITE));
    SPI.transfer(static_cast<uint8_t>(latch_register));
    SPI.transfer(bit_mask);
    ::digitalWrite(SS, HIGH);
    
    return;
}

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
