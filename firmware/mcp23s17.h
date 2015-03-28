/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */

#ifndef MCP23S17_H
#define MCP23S17_H

#include <cstdint>

class mcp23s17 {
  public:
	// Definition(s)
	enum class PinMode {
		OUTPUT = 0,
		INPUT,
	};
	
	enum class RegisterTransaction {
		WRITE = 0,
		READ,
	};
	
	/// \brief Control Registers
	/// \note IOCONA is equivalent to IOCONB
	/// \note As addressed when IOCON.BANK = 0
	enum class ControlRegister {
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
		GPIOA,
		GPIOB,
		OLATA,
		OLATB,
		REGISTER_COUNT,
	};
	
	/// \brief The hardware address of the chip
	/// \detail The chip has three pins A0, A1 and A2 dedicated
	/// to supplying an individual address to a chip, which
	/// allows up to eight devices on the bus.
	enum class HardwareAddress {
		HW_ADDR_0 = 0,
		HW_ADDR_1,
		HW_ADDR_2,
		HW_ADDR_3,
		HW_ADDR_4,
		HW_ADDR_5,
		HW_ADDR_6,
		HW_ADDR_7,
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
	// Public method(s)
	
	/// \brief Set pin mode
	/// \param [in] pin_ The number associated with the pin
	/// \param [in] mode_ The direction of use INPUT/OUTPUT
	void
	pinMode (
		const uint8_t pin_,
		const PinMode mode_
	) const;
	
  protected:
	// Protected instance variable(s)
	// Protected method(s)
	inline
	uint8_t const * const
	getControlRegister (
		void
	) const {
		return _control_register;
	}
	
	inline
	uint8_t const * const
	getControlRegisterAddresses (
		void
	) const {
		return _control_register_address;
	}
	
  private:
	// Private instance variable(s)
	const uint8_t _SPI_BUS_ADDRESS;
	uint8_t _control_register[static_cast<uint8_t>(ControlRegister::REGISTER_COUNT)];
	uint8_t _control_register_address[static_cast<uint8_t>(ControlRegister::REGISTER_COUNT)];
	
	// Private method(s)
};

#endif

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
