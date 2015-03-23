/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */

#include "MOCK_wiring.h"

MOCK_spi SPI;

const size_t ARDUINO_PINS = 14;

bool MOCK_spi::_has_begun = false;

std::function<void(void)> MOCK_spi::_begin = [](){ _has_begun = true; };
std::function<void(void)> MOCK_spi::_end = [](){};
std::function<void(uint8_t)> MOCK_spi::_setBitOrder = [](uint8_t){};
std::function<void(uint8_t)> MOCK_spi::_setClockDivider = [](uint8_t){};
std::function<void(uint8_t)> MOCK_spi::_setDataMode = [](uint8_t){};
std::function<uint8_t(uint8_t)> MOCK_spi::_transfer = [](uint8_t) -> uint8_t { return 0; };

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

namespace {
	static uint8_t _pin_latch_value[ARDUINO_PINS] = { 0 };
	static uint8_t _pin_transition[ARDUINO_PINS] = { 0 };
}

void
initMockState (
	void
) {
	MOCK_spi::_has_begun = false;
	// Set all transistions to NO_TRANSITION
	for ( int i = 0 ; i < ARDUINO_PINS ; ++i ) { _pin_transition[i] = static_cast<uint8_t>(NO_TRANSITION); }
	
	MOCK_spi::_begin = [](){
		MOCK_spi::_has_begun = true;
		_pin_latch_value[SS] = HIGH;
	};
	
	MOCK_spi::_end = [](){};
	MOCK_spi::_setBitOrder = [](uint8_t){};
	MOCK_spi::_setClockDivider = [](uint8_t){};
	MOCK_spi::_setDataMode = [](uint8_t){};
	MOCK_spi::_transfer = [](uint8_t) -> uint8_t { return 0; };
}

uint8_t
getPinLatchValue (
	const uint8_t pin_
) {
	return _pin_latch_value[pin_];
}

uint8_t
getPinTransition (
	const uint8_t pin_
) {
	return _pin_transition[pin_];
}

void
setPinLatchValue (
	const uint8_t pin_,
	const uint8_t latch_value_
) {
	if ( _pin_latch_value[pin_] != latch_value_ ) {
		_pin_transition[pin_] = latch_value_;
		_pin_latch_value[pin_] = latch_value_;
	}
}

void
digitalWrite (
	const uint8_t pin_,
	const uint8_t latch_value_
) {
	setPinLatchValue(pin_, latch_value_);
}

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
