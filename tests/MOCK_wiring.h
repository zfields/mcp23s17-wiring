/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */

#if defined(TESTING)
#ifndef MOCK_WIRING
#define MOCK_WIRING

#include <cstdint>
#include <functional>

enum BitOrder : uint8_t {
	LSBFIRST = 0,
	MSBFIRST,
};

enum ClockDivider : uint8_t {
	SPI_CLOCK_DIV4 = 0,
	SPI_CLOCK_DIV16,
	SPI_CLOCK_DIV64,
	SPI_CLOCK_DIV128,
	SPI_CLOCK_DIV2,
	SPI_CLOCK_DIV8,
	SPI_CLOCK_DIV32,
};

enum DataMode : uint8_t {
	SPI_MODE0 = 0,
	SPI_MODE1 = 4,
	SPI_MODE2 = 8,
	SPI_MODE3 = 12,
};

enum PinMode : uint8_t {
	INPUT = 0,
	OUTPUT,
	INPUT_PULLUP,
};

enum PinValue : uint8_t {
	LOW = 0,
	HIGH,
};

enum SPIPinAlias : uint8_t {
	SS = 10,
	MOSI,
	MISO,
	SCK,
};

struct MOCK_spi {
	static bool _has_begun;
	
	static std::function<void(void)> _begin;
	static std::function<void(void)> _end;
	static std::function<void(uint8_t)> _setBitOrder;
	static std::function<void(uint8_t)> _setClockDivider;
	static std::function<void(uint8_t)> _setDataMode;
	static std::function<uint8_t(uint8_t)> _transfer;
	
	static
	void
	begin (
		void
	);	
	
	static
	void
	end (
		void
	);
	
	static
	void
	setBitOrder (
		uint8_t bit_order_
	);
	
	static
	void
	setClockDivider (
		uint8_t clock_divider_
	);
	
	static
	void
	setDataMode (
		uint8_t data_mode_
	);
	
	static
	uint8_t
	transfer (
		uint8_t data_
	);	
};

void
digitalWrite (
	const uint8_t pin_,
	const uint8_t latch_value_
);

namespace MOCK {

enum class PinTransition : uint8_t {
	NO_TRANSITION = static_cast<uint8_t>(-1),
	HIGH_TO_LOW = 0,
	LOW_TO_HIGH,
};

void
initMockState (
	void
);

uint8_t
getPinLatchValue (
	const uint8_t pin_
);

PinTransition *
getPinTransition (
	const uint8_t pin_
);

void
resetPinTransitions (
	void
);

} // namespace MOCK

#endif

extern MOCK_spi SPI;

#endif

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
