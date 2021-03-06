/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../mcp23s17.h"
#include "MOCK_wiring.h"

//TODO: Consider 16-bit mode

enum class BitValue {
    UNSET = 0x00,
    SET = 0x01,
};

class TC_mcp23s17 : public mcp23s17 {
  public:
    TC_mcp23s17 (
        mcp23s17::HardwareAddress hw_addr_
    ): mcp23s17(hw_addr_)
    {}

    // Access protected test members
    using mcp23s17::getControlRegister;
    using mcp23s17::getControlRegisterAddresses;
    using mcp23s17::getInterruptServiceRoutines;
};

namespace {

class MockSPITransfer : public ::testing::Test {
  protected:
    unsigned int _index;
    uint8_t _input_latch_port;
    uint8_t *_spi_transaction;
    size_t _spi_transaction_length;

    MockSPITransfer (
        void
    ) :
        _index(0),
        _input_latch_port(0xFF),
        _spi_transaction(nullptr),
        _spi_transaction_length(8)
    {
        // This happens before SetUp()
    }
    ~MockSPITransfer () {
        // This happens after TearDown()
    }

    void SetUp (void) {
        MOCK::initMockState();
        _spi_transaction = new uint8_t[_spi_transaction_length]();
        SPI._transfer = [&](uint8_t byte_){
            _spi_transaction[_index] = byte_;
            if ( ++_index == _spi_transaction_length ) {
                return _input_latch_port;
            } else {
                return static_cast<uint8_t>(0x00);
            }
        };
    }
    void TearDown (void) {
        delete[](_spi_transaction);
    }

    void ResetSpi (size_t spi_transaction_length_ = 8) {
        _index = 0;
        _spi_transaction_length = spi_transaction_length_;
        delete[](_spi_transaction);
        _spi_transaction = new uint8_t[_spi_transaction_length]();
        MOCK::resetPinTransitions();
    }
};

/*
The first argument is the name of the test case, and the second argument
is the test's name within the test case. Both names must be valid C++
identifiers, and they should not contain underscore (_). A test's full
name consists of its containing test case and its individual name. Tests
from different test cases can have the same individual name.
(e.g. ASSERT_EQ(_EXPECTED_, _ACTUAL_))
*/
TEST(Construction, WHENObjectIsConstructedTHENAddressParameterIsStored) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    EXPECT_EQ(0x4C, gpio_x.getSpiBusAddress());
}

TEST(Construction, WHENObjectIsConstructedTHENControlRegisterAddressesArePopulatedWithBankEqualsZeroValues) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    for ( int i = 0 ; i < static_cast<int>(mcp23s17::ControlRegister::REGISTER_COUNT) ; ++i ) {
        EXPECT_EQ(i, gpio_x.getControlRegisterAddresses()[i]) << "Expected value <" << i << ">!";
    }
}

TEST(Construction, WHENObjectIsConstructedTHENControlRegisterValuesArePopulated) {
    int i = 0;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( ; i < 2 ; ++i ) {
        EXPECT_EQ(0xFF, gpio_x.getControlRegister()[i]) << "Error at index <" << i << ">!";
    }
    for ( ; i < static_cast<int>(mcp23s17::ControlRegister::REGISTER_COUNT) ; ++i ) {
        if ( static_cast<uint8_t>(mcp23s17::ControlRegister::IOCONA) == i ) {
            EXPECT_EQ(static_cast<uint8_t>(mcp23s17::IOConfigurationRegister::HAEN), gpio_x.getControlRegister()[i]) << "Error at index <" << i << ">!";
        } else {
            EXPECT_EQ(0x00, gpio_x.getControlRegister()[i]) << "Error at index <" << i << ">!";
        }
    }
}

TEST(Construction, WHENObjectIsConstructedTHENInterruptServiceRoutinesArrayIsSetToNullptr) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    for ( int i = 0 ; i < mcp23s17::PIN_COUNT ; ++i ) {
        EXPECT_EQ(nullptr, gpio_x.getInterruptServiceRoutines()[i]);
    }
}

TEST(Construction, WHENObjectIsConstructedTHENSPIBeginIsCalled) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    EXPECT_EQ(true, SPI._has_begun);
}

TEST_F(MockSPITransfer, mcp23s17$WHENObjectIsConstructedTHENTheCallersChipSelectPinIsPulledFromHighToLowAndBackOneTime) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    EXPECT_EQ(MOCK::PinTransition::HIGH_TO_LOW, MOCK::getPinTransition(SS)[0]);
    EXPECT_EQ(MOCK::PinTransition::LOW_TO_HIGH, MOCK::getPinTransition(SS)[1]);
}

TEST_F(MockSPITransfer, mcp23s17$WHENObjectIsConstructedTHENATransactionIsSentToSPIBaseAddress) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    EXPECT_EQ(static_cast<uint8_t>(mcp23s17::SPI_BASE_ADDRESS), (_spi_transaction[0] & 0xFE));
    ASSERT_LT(0, _index);
}

TEST_F(MockSPITransfer, mcp23s17$WHENObjectIsConstructedTHENAWriteTransactionIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    EXPECT_EQ(mcp23s17::RegisterTransaction::WRITE, static_cast<mcp23s17::RegisterTransaction>(_spi_transaction[0] & 0x01));
    ASSERT_LT(0, _index);
}

TEST_F(MockSPITransfer, mcp23s17$WHENObjectIsConstructedTHENTheIOCONARegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    EXPECT_EQ(mcp23s17::ControlRegister::IOCONA, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
    ASSERT_LT(1, _index);
}

TEST_F(MockSPITransfer, mcp23s17$WHENObjectIsConstructedTHENTheHAENBitIsSet) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    EXPECT_EQ(mcp23s17::IOConfigurationRegister::HAEN, static_cast<mcp23s17::IOConfigurationRegister>(_spi_transaction[2]));
    ASSERT_LT(2, _index);
}

/*
Like TEST(), the first argument is the test case name, but for TEST_F()
this must be the name of the test fixture class.
*/

  /***********/
 /* pinMode */
/***********/

TEST_F(MockSPITransfer, pinMode$WHENNotCalledTHENTheCallersChipSelectPinIsHigh) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    ResetSpi();
    EXPECT_EQ(mcp23s17::PinLatchValue::HIGH, static_cast<mcp23s17::PinLatchValue>(MOCK::getPinLatchValue(SS)));
    ASSERT_EQ(0, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForOutputTHENTheCallersChipSelectPinIsPulledFromHighToLowAndBackOneTime) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);
    EXPECT_EQ(MOCK::PinTransition::HIGH_TO_LOW, MOCK::getPinTransition(SS)[0]);
    EXPECT_EQ(MOCK::PinTransition::LOW_TO_HIGH, MOCK::getPinTransition(SS)[1]);
    ASSERT_EQ(MOCK::PinTransition::NO_TRANSITION, MOCK::getPinTransition(SS)[2]);
    ASSERT_EQ(MOCK::PinTransition::NO_TRANSITION, MOCK::getPinTransition(SS)[3]);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledTHENAWriteTransactionIsSent) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);
    EXPECT_EQ(mcp23s17::RegisterTransaction::WRITE, static_cast<mcp23s17::RegisterTransaction>(_spi_transaction[0] & 0x01));
    ASSERT_LT(0, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledOnPinLessThanEightTHENTheIODIRARegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);
        EXPECT_EQ(mcp23s17::ControlRegister::IODIRA, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
        ASSERT_LT(1, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledOnPinGreaterThanOrEqualToEightTHENTheIODIRBRegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);
        EXPECT_EQ(mcp23s17::ControlRegister::IODIRB, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
        ASSERT_LT(1, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForOutputOnPinLessThanEightTHENAMaskWithTheSpecifiedBitUnsetIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);
        EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        ASSERT_LT(2, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForOutputOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitUnsetIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);
        EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        ASSERT_LT(2, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputOnPinLessThanEightTHENAMaskWithTheSpecifiedBitSetIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        ASSERT_LT(2, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputPullupOnPinLessThanEightTHENAMaskWithTheSpecifiedBitSetIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT_PULLUP);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        ASSERT_LT(2, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitSetIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        ASSERT_LT(2, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputPullupOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitSetIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT_PULLUP);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        ASSERT_LT(2, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENOutputPinIsSetOnPortATHENItPersistsOnSubsequentCall) {
    const uint8_t PIN = 7;
    const uint8_t BIT_POSITION = PIN % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);
    ASSERT_LT(2, _index);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::OUTPUT);
    EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((gpio_x.getControlRegister()[static_cast<uint8_t>(mcp23s17::ControlRegister::IODIRA)] >> BIT_POSITION) & 0x01));
    ASSERT_LT(2, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENOutputPinIsSetOnPortBTHENItPersistsOnSubsequentCall) {
    const uint8_t PIN = 8;
    const uint8_t BIT_POSITION = PIN % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);
    ASSERT_LT(2, _index);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::OUTPUT);
    EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[2] >> BIT_POSITION) & 0x01));
    ASSERT_LT(2, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputOnPinFromTheSamePortAsAPreviousCallTHENTheOriginalPinIsNotDisturbed) {
    const uint8_t PIN = 8;
    const uint8_t BIT_POSITION = PIN % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);
    ASSERT_LT(2, _index);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::INPUT);
    EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> BIT_POSITION) & 0x01));
    ASSERT_LT(2, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputPullupOnPinFromTheSamePortAsAPreviousCallTHENTheOriginalPinIsNotDisturbed) {
    const uint8_t PIN = 8;
    const uint8_t BIT_POSITION = PIN % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);
    ASSERT_LT(2, _index);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::INPUT_PULLUP);
    EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> BIT_POSITION) & 0x01));
    ASSERT_LT(2, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledOnPinAlreadyInTheCorrectStateTHENNoSPITransactionOccurs) {
    const uint8_t PIN = 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);
    EXPECT_EQ(0, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENInputPullupIsCalledTHENTheCallersChipSelectPinIsPulledFromHighToLowAndBackTwoTimes) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT_PULLUP);
    ASSERT_EQ(MOCK::PinTransition::HIGH_TO_LOW, MOCK::getPinTransition(SS)[0]);
    ASSERT_EQ(MOCK::PinTransition::LOW_TO_HIGH, MOCK::getPinTransition(SS)[1]);
    EXPECT_EQ(MOCK::PinTransition::HIGH_TO_LOW, MOCK::getPinTransition(SS)[2]);
    EXPECT_EQ(MOCK::PinTransition::LOW_TO_HIGH, MOCK::getPinTransition(SS)[3]);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputPullupTHENAnotherWriteTransactionIsSent) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT_PULLUP);
    ASSERT_EQ(mcp23s17::RegisterTransaction::WRITE, static_cast<mcp23s17::RegisterTransaction>(_spi_transaction[0] & 0x01));
    EXPECT_EQ(mcp23s17::RegisterTransaction::WRITE, static_cast<mcp23s17::RegisterTransaction>(_spi_transaction[3] & 0x01));
    ASSERT_LT(3, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputPullupOnPinLessThanEightTHENTheGPPUARegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT_PULLUP);
        ASSERT_EQ(mcp23s17::ControlRegister::IODIRA, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
        EXPECT_EQ(mcp23s17::ControlRegister::GPPUA, static_cast<mcp23s17::ControlRegister>(_spi_transaction[4]));
        ASSERT_LT(4, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputPullupOnPinGreaterThanOrEqualToEightTHENTheGPPUBRegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT_PULLUP);
        ASSERT_EQ(mcp23s17::ControlRegister::IODIRB, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
        EXPECT_EQ(mcp23s17::ControlRegister::GPPUB, static_cast<mcp23s17::ControlRegister>(_spi_transaction[4]));
        ASSERT_LT(4, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputPullupOnPinLessThanEightTHENAMaskWithTheSpecifiedBitSetIsSentToGPPUARegister) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT_PULLUP);
        ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[5] >> bit_position) & 0x01));
        ASSERT_LT(5, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputPullupOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitSetIsSentToGPPUBRegister) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT_PULLUP);
        ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[5] >> bit_position) & 0x01));
        ASSERT_LT(5, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENInputPullupPinIsSetOnPortATHENItPersistsOnSubsequentCall) {
    const uint8_t PIN = 7;
    const uint8_t BIT_POSITION = PIN % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT_PULLUP);
    ASSERT_LT(5, _index);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::INPUT_PULLUP);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((gpio_x.getControlRegister()[static_cast<uint8_t>(mcp23s17::ControlRegister::IODIRA)] >> BIT_POSITION) & 0x01));
    EXPECT_EQ((1 << BIT_POSITION), gpio_x.getControlRegister()[static_cast<uint8_t>(mcp23s17::ControlRegister::GPPUA)]);
    ASSERT_LT(5, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENInputPullupPinIsSetOnPortBTHENItPersistsOnSubsequentCall) {
    const uint8_t PIN1 = 8;
    const uint8_t PIN2 = 10;
    const uint8_t BIT_POSITION1 = PIN1 % 8;
    const uint8_t BIT_POSITION2 = PIN2 % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN1, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN2, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN1, mcp23s17::PinMode::INPUT_PULLUP);
    ASSERT_LT(5, _index);

    ResetSpi();
    gpio_x.pinMode(PIN2, mcp23s17::PinMode::INPUT_PULLUP);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> BIT_POSITION1) & 0x01));
    EXPECT_EQ(((1 << BIT_POSITION1) | (1 << BIT_POSITION2)), _spi_transaction[5]);
    ASSERT_LT(5, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputTHENTheCallersChipSelectPinIsPulledFromHighToLowAndBackTwoTimes) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT_PULLUP);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);
    ASSERT_EQ(MOCK::PinTransition::HIGH_TO_LOW, MOCK::getPinTransition(SS)[0]);
    ASSERT_EQ(MOCK::PinTransition::LOW_TO_HIGH, MOCK::getPinTransition(SS)[1]);
    EXPECT_EQ(MOCK::PinTransition::HIGH_TO_LOW, MOCK::getPinTransition(SS)[2]);
    EXPECT_EQ(MOCK::PinTransition::LOW_TO_HIGH, MOCK::getPinTransition(SS)[3]);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputTHENAnotherWriteTransactionIsSent) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT_PULLUP);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);
    ASSERT_EQ(mcp23s17::RegisterTransaction::WRITE, static_cast<mcp23s17::RegisterTransaction>(_spi_transaction[0] & 0x01));
    EXPECT_EQ(mcp23s17::RegisterTransaction::WRITE, static_cast<mcp23s17::RegisterTransaction>(_spi_transaction[3] & 0x01));
    ASSERT_LT(3, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputOnPinLessThanEightTHENTheGPPUARegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT_PULLUP);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT);
        ASSERT_EQ(mcp23s17::ControlRegister::IODIRA, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
        EXPECT_EQ(mcp23s17::ControlRegister::GPPUA, static_cast<mcp23s17::ControlRegister>(_spi_transaction[4]));
        ASSERT_LT(4, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputOnPinGreaterThanOrEqualToEightTHENTheGPPUBRegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT_PULLUP);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT);
        ASSERT_EQ(mcp23s17::ControlRegister::IODIRB, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
        EXPECT_EQ(mcp23s17::ControlRegister::GPPUB, static_cast<mcp23s17::ControlRegister>(_spi_transaction[4]));
        ASSERT_LT(4, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputOnPinLessThanEightTHENAMaskWithTheSpecifiedBitUnsetIsSentToGPPUARegister) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT_PULLUP);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT);
        ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[5] >> bit_position) & 0x01));
        ASSERT_LT(5, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitUnsetIsSentToGPPUBRegister) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT_PULLUP);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT);
        ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[5] >> bit_position) & 0x01));
        ASSERT_LT(5, _index);
    }
}

TEST_F(MockSPITransfer, pinMode$WHENInputPinIsSetOnPortATHENInputPullupPersistsOnSubsequentCall) {
    const uint8_t PIN = 7;
    const uint8_t BIT_POSITION = PIN % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::INPUT_PULLUP);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT_PULLUP);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);
    ASSERT_LT(5, _index);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::INPUT);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((gpio_x.getControlRegister()[static_cast<uint8_t>(mcp23s17::ControlRegister::IODIRA)] >> BIT_POSITION) & 0x01));
    EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>(gpio_x.getControlRegister()[static_cast<uint8_t>(mcp23s17::ControlRegister::GPPUA)]));
    ASSERT_LT(5, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENInputPinIsSetOnPortBTHENInputPullupPersistsOnSubsequentCall) {
    const uint8_t PIN = 8;
    const uint8_t BIT_POSITION = PIN % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::INPUT_PULLUP);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT_PULLUP);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);
    ASSERT_LT(5, _index);

    ResetSpi();
    gpio_x.pinMode(10, mcp23s17::PinMode::INPUT);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> BIT_POSITION) & 0x01));
    EXPECT_EQ(0x00, _spi_transaction[5]);
    ASSERT_LT(5, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputAfterInputPullupOnSamePinTHENOnlyThePullupRegisterIsModified) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT_PULLUP);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);
    ASSERT_EQ(mcp23s17::ControlRegister::GPPUA, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
    EXPECT_EQ(0x00, _spi_transaction[2]);
    ASSERT_EQ(3, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForInputPullupAfterInputTHENThePullupRegisterUpdated) {
    const uint8_t PIN = 7;
    const uint8_t BIT_POSITION = PIN % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT_PULLUP);
    EXPECT_EQ((1 << BIT_POSITION), gpio_x.getControlRegister()[static_cast<uint8_t>(mcp23s17::ControlRegister::GPPUA)]);
    ASSERT_EQ(3, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledOnPinAlreadyInTheCorrectPullupStateTHENNoSPITransactionOccurs) {
  const uint8_t PIN = 8;
  TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT_PULLUP);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT_PULLUP);
    EXPECT_EQ(0, _index);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledOnPinGreaterThanOrEqualToPinCountTHENNoSPITransactionOccurs) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( int i = mcp23s17::PIN_COUNT ; i < 256 ; ++i ) {
        ResetSpi();
        gpio_x.pinMode(i, mcp23s17::PinMode::INPUT_PULLUP);
        EXPECT_EQ(0, _index) << "Error at index <" << i << ">!";
    }
}

  /****************/
 /* digitalWrite */
/****************/

TEST_F(MockSPITransfer, digitalWrite$WHENCalledTHENTheCallersChipSelectPinIsPulledFromHighToLowAndBack) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.digitalWrite(PIN, mcp23s17::PinLatchValue::HIGH);
    EXPECT_EQ(MOCK::PinTransition::HIGH_TO_LOW, MOCK::getPinTransition(SS)[0]);
    EXPECT_EQ(MOCK::PinTransition::LOW_TO_HIGH, MOCK::getPinTransition(SS)[1]);
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledTHENAWriteTransactionIsSent) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.digitalWrite(PIN, mcp23s17::PinLatchValue::HIGH);
    EXPECT_EQ(mcp23s17::RegisterTransaction::WRITE, static_cast<mcp23s17::RegisterTransaction>(_spi_transaction[0] & 0x01));
    ASSERT_LT(0, _index);
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledOnPinLessThanEightTHENTheGPIOARegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.digitalWrite(pin, mcp23s17::PinLatchValue::HIGH);
        EXPECT_EQ(mcp23s17::ControlRegister::GPIOA, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
        ASSERT_LT(1, _index);
    }
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledOnPinGreaterThanOrEqualToEightTHENTheGPIOBRegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.digitalWrite(pin, mcp23s17::PinLatchValue::HIGH);
        EXPECT_EQ(mcp23s17::ControlRegister::GPIOB, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
        ASSERT_LT(1, _index);
    }
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledForHighOnPinLessThanEightTHENAMaskWithTheSpecifiedBitSetIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.digitalWrite(pin, mcp23s17::PinLatchValue::HIGH);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        ASSERT_LT(2, _index);
    }
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledForHighOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitSetIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.digitalWrite(pin, mcp23s17::PinLatchValue::HIGH);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        ASSERT_LT(2, _index);
    }
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledForLowOnPinLessThanEightTHENAMaskWithTheSpecifiedBitUnsetIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.digitalWrite(pin, mcp23s17::PinLatchValue::HIGH);

        ResetSpi();
        gpio_x.digitalWrite(pin, mcp23s17::PinLatchValue::LOW);
        EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        ASSERT_LT(2, _index);
    }
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledForLowOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitUnsetIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);

        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.digitalWrite(pin, mcp23s17::PinLatchValue::HIGH);

        ResetSpi();
        gpio_x.digitalWrite(pin, mcp23s17::PinLatchValue::LOW);
        EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        ASSERT_LT(2, _index);
    }
}

TEST_F(MockSPITransfer, digitalWrite$WHENPinIsSetOnPortATHENItPersistsOnSubsequentCall) {
    const uint8_t PIN1 = 7;
    const uint8_t PIN2 = 10;
    const uint8_t BIT_POSITION1 = PIN1 % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN1, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.digitalWrite(PIN1, mcp23s17::PinLatchValue::HIGH);
    ASSERT_LT(2, _index);

    ResetSpi();
    gpio_x.pinMode(PIN2, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.digitalWrite(PIN2, mcp23s17::PinLatchValue::HIGH);
    EXPECT_EQ((1 << BIT_POSITION1), gpio_x.getControlRegister()[static_cast<uint8_t>(mcp23s17::ControlRegister::GPIOA)]);
    ASSERT_LT(2, _index);
}

TEST_F(MockSPITransfer, digitalWrite$WHENPinIsSetOnPortBTHENItPersistsOnSubsequentCall) {
    const uint8_t PIN1 = 8;
    const uint8_t PIN2 = 10;
    const uint8_t BIT_POSITION1 = PIN1 % 8;
    const uint8_t BIT_POSITION2 = PIN2 % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN1, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.digitalWrite(PIN1, mcp23s17::PinLatchValue::HIGH);
    ASSERT_LT(2, _index);

    ResetSpi();
    gpio_x.pinMode(PIN2, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.digitalWrite(PIN2, mcp23s17::PinLatchValue::HIGH);
    EXPECT_EQ((1 << BIT_POSITION1 | 1 << BIT_POSITION2), _spi_transaction[2]);
    ASSERT_LT(2, _index);
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledForLowOnPinFromTheSamePortAsAPreviousCallTHENTheOriginalPinIsNotDisturbed) {
    const uint8_t PIN1 = 8;
    const uint8_t PIN2 = 10;
    const uint8_t BIT_POSITION1 = PIN1 % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN2, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.digitalWrite(PIN2, mcp23s17::PinLatchValue::HIGH);

    ResetSpi();
    gpio_x.pinMode(PIN1, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.digitalWrite(PIN1, mcp23s17::PinLatchValue::HIGH);
    ASSERT_LT(2, _index);

    ResetSpi();
    gpio_x.digitalWrite(PIN2, mcp23s17::PinLatchValue::LOW);
    EXPECT_EQ((1 << BIT_POSITION1), _spi_transaction[2]);
    ASSERT_LT(2, _index);
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledOnPinAlreadyInTheCorrectStateTHENNoSPITransactionOccurs) {
    const uint8_t PIN = 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::OUTPUT);

    ResetSpi();
    gpio_x.digitalWrite(PIN, mcp23s17::PinLatchValue::HIGH);

    ResetSpi();
    gpio_x.digitalWrite(PIN, mcp23s17::PinLatchValue::HIGH);
    EXPECT_EQ(0, _index);
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledOnPinLessThanEightInInputModeTHENNoSPITransactionOccurs) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT);

        ResetSpi();
        gpio_x.digitalWrite(pin, mcp23s17::PinLatchValue::HIGH);
        EXPECT_EQ(0, _index);
    }
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledOnPinGreaterThanOrEqualToEightInInputModeTHENNoSPITransactionOccurs) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT);

        ResetSpi();
        gpio_x.digitalWrite(pin, mcp23s17::PinLatchValue::HIGH);
        EXPECT_EQ(0, _index);
    }
}

  /***************/
 /* digitalRead */
/***************/

TEST_F(MockSPITransfer, digitalRead$WHENCalledTHENTheCallersChipSelectPinIsPulledFromHighToLowAndBack) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);

    ResetSpi();
    gpio_x.digitalRead(PIN);
    EXPECT_EQ(MOCK::PinTransition::HIGH_TO_LOW, MOCK::getPinTransition(SS)[0]);
    EXPECT_EQ(MOCK::PinTransition::LOW_TO_HIGH, MOCK::getPinTransition(SS)[1]);
}

TEST_F(MockSPITransfer, digitalRead$WHENCalledTHENAReadTransactionIsSent) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);

    ResetSpi();
    gpio_x.digitalRead(PIN);
    EXPECT_EQ(mcp23s17::RegisterTransaction::READ, static_cast<mcp23s17::RegisterTransaction>(_spi_transaction[0] & 0x01));
    ASSERT_LT(0, _index);
}

TEST_F(MockSPITransfer, digitalRead$WHENCalledOnPinLessThanEightTHENTheGPIOARegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT);

        ResetSpi();
        gpio_x.digitalRead(pin);
        EXPECT_EQ(mcp23s17::ControlRegister::GPIOA, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
        ASSERT_LT(1, _index);
    }
}

TEST_F(MockSPITransfer, digitalRead$WHENCalledOnPinGreaterThanOrEqualToEightTHENTheGPIOBRegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::INPUT);

        ResetSpi();
        gpio_x.digitalRead(pin);
        EXPECT_EQ(mcp23s17::ControlRegister::GPIOB, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
        ASSERT_LT(1, _index);
    }
}

TEST_F(MockSPITransfer, digitalRead$WHENCalledTHENAFlushBitIsSent) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);

    ResetSpi();
    gpio_x.digitalRead(PIN);
    EXPECT_LT(2, _index);
}

TEST_F(MockSPITransfer, digitalRead$WHENCalledOnPinLessThanEightInOutputModeTHENNoSPITransactionOccurs) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.digitalRead(pin);
        EXPECT_EQ(0, _index);
    }
}

TEST_F(MockSPITransfer, digitalRead$WHENCalledOnPinLessThanEightInOutputModeTHENLOWIsReturned) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        EXPECT_EQ(mcp23s17::PinLatchValue::LOW, gpio_x.digitalRead(pin));
        ASSERT_EQ(0, _index);
    }
}

TEST_F(MockSPITransfer, digitalRead$WHENCalledOnPinGreaterThanOrEqualToEightInInputModeTHENNoSPITransactionOccurs) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        gpio_x.digitalRead(pin);
        EXPECT_EQ(0, _index);
    }
}

TEST_F(MockSPITransfer, digitalRead$WHENCalledOnPinGreaterThanOrEqualToEightInInputModeTHENLOWIsReturned) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        ResetSpi();
        gpio_x.pinMode(pin, mcp23s17::PinMode::OUTPUT);

        ResetSpi();
        EXPECT_EQ(mcp23s17::PinLatchValue::LOW, gpio_x.digitalRead(pin));
        ASSERT_EQ(0, _index);
    }
}

TEST_F(MockSPITransfer, digitalRead$WHENCalledOnLOWPinTHENLOWIsReturned) {
    const uint8_t PIN = 8;
    // Index: 76543210
    // Value: 11001010
    _input_latch_port = 0xCA;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);

    ResetSpi();
    EXPECT_EQ(mcp23s17::PinLatchValue::LOW, gpio_x.digitalRead(PIN));
    ASSERT_LT(2, _index);
}

TEST_F(MockSPITransfer, digitalRead$WHENCalledOnHIGHPinTHENHIGHIsReturned) {
    const uint8_t PIN = 3;
    // Index: 76543210
    // Value: 11001010
    _input_latch_port = 0xCA;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);

    ResetSpi();
    gpio_x.pinMode(PIN, mcp23s17::PinMode::INPUT);

    ResetSpi(3);
    EXPECT_EQ(mcp23s17::PinLatchValue::HIGH, gpio_x.digitalRead(PIN));
    ASSERT_LT(2, _index);
}

  /*******************/
 /* attachInterrupt */
/*******************/

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledTHENCorrespondingCallbackIsStored) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ASSERT_EQ(nullptr, gpio_x.getInterruptServiceRoutines()[PIN]);
    gpio_x.attachInterrupt(PIN, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);

    EXPECT_EQ(interrupt_service_routine, gpio_x.getInterruptServiceRoutines()[PIN]);
}
/*
TEST_F(MockSPITransfer, attachInterrupt$WHENCalledWithPinGreaterThanOrEqualToPinCountTHENInterruptServiceRoutineArrayIsNotModified) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( int i = 0 ; i < mcp23s17::PIN_COUNT ; ++i ) {
        gpio_x.attachInterrupt(i, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    }

    for ( int i = mcp23s17::PIN_COUNT ; i < 256 ; ++i ) {
        gpio_x.attachInterrupt(i, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
        for ( int j = 0 ; j < mcp23s17::PIN_COUNT ; ++j ) {
            EXPECT_EQ(interrupt_service_routine, gpio_x.getInterruptServiceRoutines()[j]) << "Error at index <" << j << ">!";
        }
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledWithNullFunctionPointerTHENInterruptServiceRoutineArrayIsNotModified) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( int i = 0 ; i < mcp23s17::PIN_COUNT ; ++i ) {
        gpio_x.attachInterrupt(i, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    }

    for ( int i = 0 ; i < mcp23s17::PIN_COUNT ; ++i ) {
        gpio_x.attachInterrupt(i, nullptr, mcp23s17::InterruptMode::HIGH);
        EXPECT_EQ(interrupt_service_routine, gpio_x.getInterruptServiceRoutines()[i]) << "Error at index <" << i << ">!";
    }
}
*/
TEST_F(MockSPITransfer, attachInterrupt$WHENCalledTHENTheCallersChipSelectPinIsPulledFromHighToLowAndBackOneTime) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    EXPECT_EQ(MOCK::PinTransition::HIGH_TO_LOW, MOCK::getPinTransition(SS)[0]);
    EXPECT_EQ(MOCK::PinTransition::LOW_TO_HIGH, MOCK::getPinTransition(SS)[1]);
    ASSERT_EQ(MOCK::PinTransition::NO_TRANSITION, MOCK::getPinTransition(SS)[2]);
    ASSERT_EQ(MOCK::PinTransition::NO_TRANSITION, MOCK::getPinTransition(SS)[3]);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledTHENATransactionIsSentToTheHardwareAddress) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    EXPECT_EQ(gpio_x.getSpiBusAddress(), (_spi_transaction[0] & 0xFE));
    ASSERT_LT(0, _index);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledTHENAWriteTransactionIsSent) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    EXPECT_EQ(mcp23s17::RegisterTransaction::WRITE, static_cast<mcp23s17::RegisterTransaction>(_spi_transaction[0] & 0x01));
    ASSERT_LT(0, _index);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledTHENTheGPINTENARegisterIsTargeted) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    EXPECT_EQ(mcp23s17::ControlRegister::GPINTENA, static_cast<mcp23s17::ControlRegister>(_spi_transaction[1]));
    ASSERT_LT(1, _index);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledTHENTwoControlBytesAndSixBytesOfDataAreWritten) {
    const uint8_t PIN = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    ASSERT_LT(7, _index);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledOnPinLessThanEightTHENAMaskWithTheSpecifiedBitSetIsSentToGPINTENA) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledForHIGHOnPinLessThanEightTHENAMaskWithTheSpecifiedBitSetIsSentToDEFVALA) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[4] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledForHIGHOnPinLessThanEightTHENAMaskWithTheSpecifiedBitSetIsSentToINTCONA) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitSetIsSentToGPINTENB) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[3] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledForHIGHOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitSetIsSentToDEFVALB) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[5] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledForHIGHOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitSetIsSentToINTCONB) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[7] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledForCHANGEOnPinLessThanEightTHENAMaskWithTheSpecifiedBitUnsetIsSentToINTCONA) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::CHANGE);
        EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[6] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledForCHANGEOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitUnsetIsSentToINTCONB) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::CHANGE);
        EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[7] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledForLOWOnPinLessThanEightTHENAMaskWithTheSpecifiedBitUnsetIsSentToDEFVALA) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
        EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[4] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledForLOWOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitUnsetIsSentToDEFVALB) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
        EXPECT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[5] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledForLOWOnPinLessThanEightTHENAMaskWithTheSpecifiedBitSetIsSentToINTCONA) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 0 ; pin < 8 ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENCalledForLOWOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitSetIsSentToINTCONB) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    for ( uint8_t pin = 8 ; pin < mcp23s17::PIN_COUNT ; ++pin ) {
        uint8_t bit_position = (pin % 8);
        ResetSpi();
        gpio_x.attachInterrupt(pin, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
        EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[7] >> bit_position) & 0x01));
        ASSERT_LT(7, _index);
    }
}

TEST_F(MockSPITransfer, attachInterrupt$WHENEnablePinIsSetOnPortATHENItPersistsOnSubsequentCall) {
    const uint8_t PIN1 = 3;
    const uint8_t PIN2 = 5;
    const uint8_t BIT_POSITION1 = (PIN1 % 8);
    const uint8_t BIT_POSITION2 = (PIN2 % 8);
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN1, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);

    ResetSpi();
    gpio_x.attachInterrupt(PIN2, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> BIT_POSITION2) & 0x01));
    EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENEnablePinIsSetOnPortBTHENItPersistsOnSubsequentCall) {
    const uint8_t PIN1 = 8;
    const uint8_t PIN2 = 3;
    const uint8_t BIT_POSITION1 = (PIN1 % 8);
    const uint8_t BIT_POSITION2 = (PIN2 % 8);
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN1, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[3] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);

    ResetSpi();
    gpio_x.attachInterrupt(PIN2, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[2] >> BIT_POSITION2) & 0x01));
    EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[3] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENInterruptModeIsSetToHighOnPortATHENControlPinPersistsOnSubsequentCall) {
    const uint8_t PIN1 = 3;
    const uint8_t PIN2 = 5;
    const uint8_t BIT_POSITION1 = (PIN1 % 8);
    const uint8_t BIT_POSITION2 = (PIN2 % 8);
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN1, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);

    ResetSpi();
    gpio_x.attachInterrupt(PIN2, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION2) & 0x01));
    EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENInterruptModeIsSetToHighOnPortBTHENControlPinPersistsOnSubsequentCall) {
    const uint8_t PIN1 = 8;
    const uint8_t PIN2 = 3;
    const uint8_t BIT_POSITION1 = (PIN1 % 8);
    const uint8_t BIT_POSITION2 = (PIN2 % 8);
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN1, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[7] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);

    ResetSpi();
    gpio_x.attachInterrupt(PIN2, interrupt_service_routine, mcp23s17::InterruptMode::HIGH);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION2) & 0x01));
    EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[7] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENInterruptModeIsSetToLowOnPortATHENControlPinPersistsOnSubsequentCall) {
    const uint8_t PIN1 = 3;
    const uint8_t PIN2 = 5;
    const uint8_t BIT_POSITION1 = (PIN1 % 8);
    const uint8_t BIT_POSITION2 = (PIN2 % 8);
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN1, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);

    ResetSpi();
    gpio_x.attachInterrupt(PIN2, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION2) & 0x01));
    EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENInterruptModeIsSetToLowOnPortBTHENControlPinPersistsOnSubsequentCall) {
    const uint8_t PIN1 = 8;
    const uint8_t PIN2 = 3;
    const uint8_t BIT_POSITION1 = (PIN1 % 8);
    const uint8_t BIT_POSITION2 = (PIN2 % 8);
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN1, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[7] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);

    ResetSpi();
    gpio_x.attachInterrupt(PIN2, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION2) & 0x01));
    EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[7] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENInterruptModeIsSetToChangeOnPortATHENControlPinPersistsOnSubsequentCall) {
    const uint8_t PIN1 = 3;
    const uint8_t PIN2 = 5;
    const uint8_t BIT_POSITION1 = (PIN1 % 8);
    const uint8_t BIT_POSITION2 = (PIN2 % 8);
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN1, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);

    ResetSpi();
    gpio_x.attachInterrupt(PIN2, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION2) & 0x01));
    ASSERT_LT(7, _index);

    ResetSpi();
    gpio_x.attachInterrupt(PIN2, interrupt_service_routine, mcp23s17::InterruptMode::CHANGE);
    ASSERT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION2) & 0x01));
    EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);
}

TEST_F(MockSPITransfer, attachInterrupt$WHENInterruptModeIsSetToChangeOnPortBTHENControlPinPersistsOnSubsequentCall) {
    const uint8_t PIN1 = 8;
    const uint8_t PIN2 = 3;
    const uint8_t BIT_POSITION1 = (PIN1 % 8);
    const uint8_t BIT_POSITION2 = (PIN2 % 8);
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    mcp23s17::isr_t interrupt_service_routine = [](){};

    ResetSpi();
    gpio_x.attachInterrupt(PIN1, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[7] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);

    ResetSpi();
    gpio_x.attachInterrupt(PIN2, interrupt_service_routine, mcp23s17::InterruptMode::LOW);
    ASSERT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION2) & 0x01));
    ASSERT_LT(7, _index);

    ResetSpi();
    gpio_x.attachInterrupt(PIN2, interrupt_service_routine, mcp23s17::InterruptMode::CHANGE);
    ASSERT_EQ(BitValue::UNSET, static_cast<BitValue>((_spi_transaction[6] >> BIT_POSITION2) & 0x01));
    EXPECT_EQ(BitValue::SET, static_cast<BitValue>((_spi_transaction[7] >> BIT_POSITION1) & 0x01));
    ASSERT_LT(7, _index);
}

//TODO: invokeInterruptServiceRoutine() - Function to call interrupt routines upon interrupt from MCP23S17
} // namespace
/*
int main (int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
*/

/* Created and copyrighted by Zachary J. Fields. Offered as open source under the MIT License (MIT). */
