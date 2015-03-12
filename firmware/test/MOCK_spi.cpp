/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */

#include "MOCK_spi.h"

void
MOCK_spi::begin (
	void
) {
	return _begin(void);
}

void
MOCK_spi::end (
	void
) {
	return _end(void);
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

MOCK_spi SPI;

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
