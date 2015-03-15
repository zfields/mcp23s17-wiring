/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */

#if defined(TESTING)
#ifndef MOCK_SPI
#define MOCK_SPI

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
/*
struct SPISettings {
	
};
*/
struct MOCK_spi {
	static
	void
	begin (
		void
	);
/*	
	static
	void
	beginTransaction (
		SPISettings settings_
	);
*/	
	static
	void
	end (
		void
	);
/*	
	static
	void
	endTransaction (
		void
	);
*/	
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
	
	static std::function<void(void)> _begin;
	static std::function<void(void)> _end;
	static std::function<void(uint8_t)> _setBitOrder;
	static std::function<void(uint8_t)> _setClockDivider;
	static std::function<void(uint8_t)> _setDataMode;
	static std::function<uint8_t(uint8_t)> _transfer;
};

#endif

extern MOCK_spi SPI;

#endif

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
