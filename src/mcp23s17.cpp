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
    _SPI_BUS_ADDRESS(SPI_BASE_ADDRESS | (static_cast<uint8_t>(hw_addr_) << 1)),
    _control_register{ 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    _control_register_address{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 },
    _interrupt_service_routines{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
{
    ::SPI.begin();

    //TODO: Load cache from chip registers (requires special handling if IOCON:HAEN is unset, or IOCON:BANK is set), or use a capture a reset pin so the chip can be put in a known state.

    // Set IOCON:HAEN bit
    _control_register[static_cast<uint8_t>(mcp23s17::ControlRegister::IOCONA)] |= static_cast<uint8_t>(IOConfigurationRegister::HAEN);

    ::digitalWrite(SS, LOW);
    ::SPI.transfer(SPI_BASE_ADDRESS);
    ::SPI.transfer(static_cast<uint8_t>(ControlRegister::IOCONA));
    ::SPI.transfer(static_cast<uint8_t>(IOConfigurationRegister::HAEN));
    ::digitalWrite(SS, HIGH);

    return;
}

void
mcp23s17::attachInterrupt (
    const uint8_t pin_,
    const isr_t interrupt_service_routine_,
    const InterruptMode mode_
) {
    uint16_t interrupt_control_cache(0x0000);
    uint16_t interrupt_enable_cache(0x0000);

    //if ( pin_ >= PIN_COUNT ) { return; }
    //if ( !interrupt_service_routine_ ) { return; }
    _interrupt_service_routines[pin_] = interrupt_service_routine_;

    // Check enable cache for existing data
    interrupt_enable_cache = _control_register[static_cast<uint8_t>(ControlRegister::GPINTENA)];
    interrupt_enable_cache |= (_control_register[static_cast<uint8_t>(ControlRegister::GPINTENB)] << 8);
    interrupt_enable_cache |= (1 << pin_);
    _control_register[static_cast<uint8_t>(ControlRegister::GPINTENA)] = interrupt_enable_cache;
    _control_register[static_cast<uint8_t>(ControlRegister::GPINTENB)] = (interrupt_enable_cache >> 8);

    // Check control cache for existing data
    interrupt_control_cache = _control_register[static_cast<uint8_t>(ControlRegister::INTCONA)];
    interrupt_control_cache |= (_control_register[static_cast<uint8_t>(ControlRegister::INTCONB)] << 8);
    if ( InterruptMode::CHANGE != mode_ ) {
        interrupt_control_cache |= (1 << pin_);
    } else {
        interrupt_control_cache &= ~(1 << pin_);
    }
    _control_register[static_cast<uint8_t>(ControlRegister::INTCONA)] = interrupt_control_cache;
    _control_register[static_cast<uint8_t>(ControlRegister::INTCONB)] = (interrupt_control_cache >> 8);

    // Three bytes are required to update a single register. Therefore, if a comparison-based interrupt is requested, then both ports of all three registers are written at once to optimize the transfer by one byte. Otherwise, if a change-based interrupt is requested, then it is more efficient to write two transactions to the to the specific ports and registers.
    ::digitalWrite(SS, LOW);
    ::SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::WRITE));
    ::SPI.transfer(static_cast<uint8_t>(ControlRegister::GPINTENA));
    ::SPI.transfer(interrupt_enable_cache);  // GPINTENA
    ::SPI.transfer(interrupt_enable_cache >> 8);  // GPINTENB
    if ( InterruptMode::HIGH == mode_ ) {
        ::SPI.transfer(1 << pin_);  // DEFVALA
        ::SPI.transfer(1 << (pin_ % 8));  // DEFVALB
        ::SPI.transfer(interrupt_control_cache);  // INTCONA
        ::SPI.transfer(interrupt_control_cache >> 8);  // INTCONB
    } else {
        ::SPI.transfer(0x00);  // DEFVALA
        ::SPI.transfer(0x00);  // DEFVALB
        if ( InterruptMode::CHANGE == mode_ ) {
            ::SPI.transfer(interrupt_control_cache);  // INTCONA
            ::SPI.transfer(interrupt_control_cache >> 8);  // INTCONB
        } else {
            ::SPI.transfer(interrupt_control_cache);  // INTCONA
            ::SPI.transfer(interrupt_control_cache >> 8);  // INTCONB
        }
    }
    ::digitalWrite(SS, HIGH);
}

mcp23s17::PinLatchValue
mcp23s17::digitalRead (
    const uint8_t pin_
) const {
    const unsigned int bit_pos(pin_ % 8);

    ControlRegister latch_register(ControlRegister::GPIOA_);
    ControlRegister direction_register(ControlRegister::IODIRA);
    uint8_t port_latch_values(0x00);

    // Select the appropriate port
    if ( pin_ / 8 ) {
        latch_register = ControlRegister::GPIOB_;
        direction_register = ControlRegister::IODIRB;
    }

    // Check to see if device is in the proper state
    if ( PinMode::OUTPUT == static_cast<PinMode>((_control_register[static_cast<uint8_t>(direction_register)] >> bit_pos) & 0x01) ) { return PinLatchValue::LOW; }

    // Send data
    ::digitalWrite(SS, LOW);
    ::SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::READ));
    ::SPI.transfer(static_cast<uint8_t>(latch_register));
    port_latch_values = ::SPI.transfer(static_cast<uint8_t>(latch_register));  // Arbitrary bit to flush result buffer. `latch_register` is selected, because it is guaranteed to be in active memory.
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

    ControlRegister latch_register(ControlRegister::GPIOA_);
    ControlRegister direction_register(ControlRegister::IODIRA);
    uint8_t registry_value;

    // Select the appropriate port
    if ( pin_ / 8 ) {
        latch_register = ControlRegister::GPIOB_;
        direction_register = ControlRegister::IODIRB;
    }

    // Check to see if device is in the proper state
    if ( PinMode::INPUT == static_cast<PinMode>((_control_register[static_cast<uint8_t>(direction_register)] >> bit_pos) & 0x01) ) { return; }

    // Check cache for existing data
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
    ::SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::WRITE));
    ::SPI.transfer(static_cast<uint8_t>(latch_register));
    ::SPI.transfer(registry_value);
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
    ControlRegister pullup_register(ControlRegister::GPPUA);
    uint8_t latch_register_cache;
    uint8_t pullup_register_cache;

    // Select the appropriate port
    if ( pin_ / 8 ) {
        latch_register = ControlRegister::IODIRB;
        pullup_register = ControlRegister::GPPUB;
    }

    // Check cache for existing data
    latch_register_cache = _control_register[static_cast<uint8_t>(latch_register)];
    pullup_register_cache = _control_register[static_cast<uint8_t>(pullup_register)];
    switch ( mode_ ) {
      case PinMode::OUTPUT:
        latch_register_cache &= ~bit_mask;
        break;
      case PinMode::INPUT:
        pullup_register_cache &= ~bit_mask;
        latch_register_cache |= bit_mask;
        break;
      case PinMode::INPUT_PULLUP:
        pullup_register_cache |= bit_mask;
        latch_register_cache |= bit_mask;
        break;
    }

    //TODO: Move check up while creating tests to ensure no other state is modified
    if ( pin_ >= PIN_COUNT ) { return; }

    // Send data to IODIR[A|B] registers, if necessary
    if ( _control_register[static_cast<uint8_t>(latch_register)] != latch_register_cache ) {
        _control_register[static_cast<uint8_t>(latch_register)] = latch_register_cache;

        ::digitalWrite(SS, LOW);
        ::SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::WRITE));
        ::SPI.transfer(static_cast<uint8_t>(latch_register));
        ::SPI.transfer(latch_register_cache);
        ::digitalWrite(SS, HIGH);
    }

    // Send data to GPPU[A|B] registers, if necessary
    if ( _control_register[static_cast<uint8_t>(pullup_register)] != pullup_register_cache ) {
        _control_register[static_cast<uint8_t>(pullup_register)] = pullup_register_cache;

        ::digitalWrite(SS, LOW);
        ::SPI.transfer(_SPI_BUS_ADDRESS | static_cast<uint8_t>(RegisterTransaction::WRITE));
        ::SPI.transfer(static_cast<uint8_t>(pullup_register));
        ::SPI.transfer(pullup_register_cache);
        ::digitalWrite(SS, HIGH);
    }

    return;
}

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
