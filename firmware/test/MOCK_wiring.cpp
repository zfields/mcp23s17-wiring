/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */

#include "MOCK_wiring.h"

MOCK_spi SPI;

std::function<void(void)> MOCK_spi::_begin = [](){ _has_begun = true; };
std::function<void(void)> MOCK_spi::_end = [](){};
std::function<void(uint8_t)> MOCK_spi::_setBitOrder = [](uint8_t){};
std::function<void(uint8_t)> MOCK_spi::_setClockDivider = [](uint8_t){};
std::function<void(uint8_t)> MOCK_spi::_setDataMode = [](uint8_t){};
std::function<uint8_t(uint8_t)> MOCK_spi::_transfer = [](uint8_t) -> uint8_t { return 0; };
bool MOCK_spi::_has_begun = false;

void
MOCK_spi::begin (
	void
) {
	return _begin();
}

void
MOCK_spi::end (
	void
) {
	return _end();
}

void
MOCK_spi::setBitOrder (
	uint8_t bit_order_
) {
	return _setBitOrder(bit_order_);
}

void
MOCK_spi::setClockDivider (
	uint8_t clock_divider_
) {
	return _setClockDivider(clock_divider_);
}

void
MOCK_spi::setDataMode (
	uint8_t data_mode_
) {
	return _setDataMode(data_mode_);
}

uint8_t
MOCK_spi::transfer (
	uint8_t data_
) {
	return _transfer(data_);
}

void
initMockState (
	void
) {
	MOCK_spi::_begin = [](){ MOCK_spi::_has_begun = true; };
	MOCK_spi::_end = [](){};
	MOCK_spi::_setBitOrder = [](uint8_t){};
	MOCK_spi::_setClockDivider = [](uint8_t){};
	MOCK_spi::_setDataMode = [](uint8_t){};
	MOCK_spi::_transfer = [](uint8_t) -> uint8_t { return 0; };
	MOCK_spi::_has_begun = false;
}

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
