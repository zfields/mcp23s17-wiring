/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */

#ifndef MCP23S17_H
#define MCP23S17_H

#include <cstdint>

class mcp23s17 {
  public:
    // Definition(s)
    typedef void(*isr_t)(void);

    /// \brief Control Registers
    /// \note IOCONA is equivalent to IOCONB
    /// \note As addressed when IOCON.BANK = 0
    enum class ControlRegister : uint8_t {
        IODIRA = 0,
        IODIRB,
        IPOLA,
        IPOLB,
        GPINTENA,
        GPINTENB,
        DEFVALA,
        DEFVALB,
        INTCONA,
        INTCONB,
        IOCONA,
        IOCONB,
        GPPUA,
        GPPUB,
        INTFA,
        INTFB,
        INTCAPA,
        INTCAPB,
        GPIOA_,
        GPIOB_,
        OLATA,
        OLATB,
        REGISTER_COUNT,
    };

    /// \brief The hardware address of the chip
    /// \detail The chip has three pins A0, A1 and A2 dedicated
    /// to supplying an individual address to a chip, which
    /// allows up to eight devices on the bus.
    enum class HardwareAddress : uint8_t {
        HW_ADDR_0 = 0,
        HW_ADDR_1,
        HW_ADDR_2,
        HW_ADDR_3,
        HW_ADDR_4,
        HW_ADDR_5,
        HW_ADDR_6,
        HW_ADDR_7,
    };

    /// \brief Interrupt Mode
    enum class InterruptMode {
        LOW = 0,
        HIGH = 1,
        CHANGE,
        FALLING,
        RISING,
    };

    /// \brief I/O Control Register
    /// \n Unimplemented (bit 0) - Read as ‘0’.
    /// \n INTPOL (bit 1) - This bit sets the polarity of the INT output pin.
    /// \n - 0 = Active-low.
    /// \n - 1 = Active-high.
    /// \n ODR (bit 2) - This bit configures the INT pin as an open-drain output.
    /// \n - 0 = Active driver output (INTPOL bit sets the polarity).
    /// \n - 1 = Open-drain output (overrides the INTPOL bit).
    /// \n HAEN (bit 3) - Hardware Address Enable bit (MCP23S17 only, address pins are always enabled on MCP23017).
    /// \n - 0 = Disables the MCP23S17 address pins.
    /// \n - 1 = Enables the MCP23S17 address pins.
    /// \n DISSLW (bit 4) - Slew Rate control bit for SDA output.
    /// \n - 0 = Slew rate enabled.
    /// \n - 1 = Slew rate disabled.
    /// \n SEQOP (bit 5) - Sequential Operation mode bit.
    /// \n - 0 = Sequential operation enabled, address pointer increments.
    /// \n - 1 = Sequential operation disabled, address pointer does not increment.
    /// \n MIRROR (bit 6) - INT Pins Mirror bit
    /// \n - 0 = The INT pins are not connected. INTA is associated with PortA and INTB is associated with PortB
    /// \n - 1 = The INT pins are internally connected
    /// \n BANK (bit 7) - Controls how the registers are addressed
    /// \n - 0 = The registers are in the same bank (addresses are sequential)
    /// \n - 1 = The registers associated with each port are separated into different banks
    enum class IOConfigurationRegister : uint8_t {
        UNIMPLEMENTED = 0x01,
        INTPOL = 0x02,
        ODR = 0x04,
        HAEN = 0x08,
        DISSLW = 0x10,
        SEQOP = 0x20,
        MIRROR = 0x40,
        BANK = 0x80,
    };

    /// \brief Pin Latch Value
    /// \note HIGH => Vcc
    /// \n LOW => GND
    enum class PinLatchValue {
        LOW = 0,
        HIGH,
    };

    /// \brief Pin Modes
    /// \note INPUT_PULLUP uses an internal 100kΩ resistor
    /// \note The pull-up resistor will provide 40-115uA of current
    enum class PinMode {
        OUTPUT = 0,
        INPUT,
        INPUT_PULLUP,
    };

    /// \brief Register Transaction Flag
    enum class RegisterTransaction : uint8_t {
        WRITE = 0,
        READ,
    };

    // Constructor and destructor method(s)

    /// \brief Object Constructor
    /// \param [in] hw_addr_ The hardware address of the device
    mcp23s17 (
        const HardwareAddress hw_addr_
    );

    // Accessor method(s)

    /// \brief Hardware address of device
    /// \return Hardware address of the device
    inline
    uint8_t
    getSpiBusAddress (
        void
    ) const {
        return _SPI_BUS_ADDRESS;
    }

    // Public instance variable(s)
    static const uint8_t PIN_COUNT = 16;
    static const uint8_t SPI_BASE_ADDRESS = 0x40;

    // Public method(s)

    /// \brief Attach interrupt to specified pin
    /// \param [in] pin_ The number associated with the pin
    /// \param [in] interrupt_service_routine_ The callback to be fired when the interrupt occurs
    /// \param [in] mode_ The mode of the interrupt
    /// \n - CHANGE - Signal when pin state changes from previous state
    /// \n - FALLING - Signal when pin transistions from HIGH to LOW
    /// \n - HIGH - Signal when pin state is HIGH
    /// \n - LOW - Signal when pin state is LOW
    /// \n - RISING - Signal when pin transistions from LOW to HIGH
    void
    attachInterrupt (
        const uint8_t pin_,
        const isr_t interrupt_service_routine_,
        const InterruptMode mode_
    );

    /// \brief Read from GPIO pins
    /// \param [in] pin_ The number associated with the pin
    /// \return HIGH or LOW based on the voltage level on the pin
    PinLatchValue
    digitalRead (
        const uint8_t pin_
    ) const;

    /// \brief Write HIGH or LOW on pins
    /// \param [in] pin_ The number associated with the pin
    /// \param [in] value_ The value set to the latch
    /// \n - HIGH => Vcc
    /// \n - LOW => GND
    void
    digitalWrite (
        const uint8_t pin_,
        const PinLatchValue value_
    );

    /// \brief Set pin mode
    /// \param [in] pin_ The number associated with the pin
    /// \param [in] mode_ The direction to set the GPIO pins
    /// \n - INPUT => Prepares the chip to read from the GPIO pins
    /// \n - INPUT_PULLUP => Prepares the chip to read from the GPIO pins,
    /// and supplies a 100kΩ resistor to pull the line HIGH while idle.
    /// \n - OUTPUT => Prepares the chip to write from the GPIO pins
    void
    pinMode (
        const uint8_t pin_,
        const PinMode mode_
    );

  protected:
    // Protected instance variable(s)
    // Protected method(s)
    inline
    uint8_t const *
    getControlRegister (
        void
    ) const {
        return _control_register;
    }

    inline
    uint8_t const *
    getControlRegisterAddresses (
        void
    ) const {
        return _control_register_address;
    }

    inline
    isr_t const *
    getInterruptServiceRoutines (
        void
    ) const {
        return _interrupt_service_routines;
    }

  private:
    // Private instance variable(s)
    const uint8_t _SPI_BUS_ADDRESS;
    uint8_t _control_register[static_cast<uint8_t>(ControlRegister::REGISTER_COUNT)];
    uint8_t _control_register_address[static_cast<uint8_t>(ControlRegister::REGISTER_COUNT)];
    isr_t _interrupt_service_routines[PIN_COUNT];

    // Private method(s)
};

#endif

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
