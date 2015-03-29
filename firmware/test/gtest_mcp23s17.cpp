/* Created and copyrighted by Zachary J. Fields. All rights reserved. */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../mcp23s17.h"
#include "MOCK_wiring.h"

class TC_mcp23s17 : public mcp23s17 {
  public:
    TC_mcp23s17 (
        mcp23s17::HardwareAddress hw_addr_
    ): mcp23s17(hw_addr_)
    {}
    
    // Access protected test members
    using mcp23s17::getControlRegister;
    using mcp23s17::getControlRegisterAddresses;
};

namespace {

//TODO: Split out PinMode fixture from SPI
class MockSPITransfer : public ::testing::Test {
  protected:
    uint8_t _spi_transaction[3];
    int _index;
    
    MockSPITransfer (
        void
    ) :
        _index(0)
    {
        // This happens before SetUp()
    }
    ~MockSPITransfer() {
        // This happens after TearDown()
    }
    
    void SetUp (void) {
        MOCK::initMockState();
        for ( int i = 0 ; i < sizeof(_spi_transaction) ; ++ i ) { _spi_transaction[i] = 0x00; }
        SPI._transfer = [&](uint8_t byte_){
            _spi_transaction[_index] = byte_;
            ++_index;
            return 0x00;
        };
    }
    void TearDown (void) {}
   
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
        EXPECT_EQ(0x00, gpio_x.getControlRegister()[i]) << "Error at index <" << i << ">!";
    }
}

TEST(Construction, WHENObjectIsConstructedTHENSPIBeginIsCalled) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    EXPECT_EQ(true, SPI._has_begun);
}

//TODO: Query state from device to init cache

/*
Like TEST(), the first argument is the test case name, but for TEST_F()
this must be the name of the test fixture class.
*/

TEST_F(MockSPITransfer, pinMode$WHENNotCalledTHENTheCallersChipSelectPinIsHigh) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    EXPECT_EQ(HIGH, MOCK::getPinLatchValue(SS));
}

TEST_F(MockSPITransfer, pinMode$WHENCalledTHENTheCallersChipSelectPinIsPulledFromHighToLowAndBack) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    gpio_x.pinMode(3, mcp23s17::PinMode::OUTPUT);
    EXPECT_EQ(MOCK::PinTransition::LOW_TO_HIGH, MOCK::getPinTransition(SS));
}

TEST_F(MockSPITransfer, pinMode$WHENCalledTHENAWriteTransactionIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    gpio_x.pinMode(3, mcp23s17::PinMode::OUTPUT);
    ASSERT_LT(0, _index);
    EXPECT_EQ(static_cast<uint8_t>(mcp23s17::RegisterTransaction::WRITE), (_spi_transaction[0] & 0x01));
}

TEST_F(MockSPITransfer, pinMode$WHENCalledOnPinLessThanEightTHENTheIODIRARegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    gpio_x.pinMode(3, mcp23s17::PinMode::OUTPUT);
    ASSERT_LT(1, _index);
    EXPECT_EQ(static_cast<uint8_t>(mcp23s17::ControlRegister::IODIRA), _spi_transaction[1]);
}

//TODO: Consider 16-bit mode
TEST_F(MockSPITransfer, pinMode$WHENCalledOnPinGreaterThanOrEqualToEightTHENTheIODIRBRegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    gpio_x.pinMode(8, mcp23s17::PinMode::OUTPUT);
    ASSERT_LT(1, _index);
    EXPECT_EQ(static_cast<uint8_t>(mcp23s17::ControlRegister::IODIRB), _spi_transaction[1]);
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForOutputOnPinLessThanEightTHENAMaskWithTheSpecifiedBitUnsetIsSent) {
    const uint8_t BIT_POSITION = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    gpio_x.pinMode(3, mcp23s17::PinMode::OUTPUT);
    ASSERT_LT(2, _index);
    EXPECT_EQ(0x00, ((_spi_transaction[2] >> BIT_POSITION) & 0x01));
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForOutputOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitUnsetIsSent) {
    const uint8_t BIT_POSITION = 8 % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    gpio_x.pinMode(8, mcp23s17::PinMode::OUTPUT);
    ASSERT_LT(2, _index);
    EXPECT_EQ(0x00, ((_spi_transaction[2] >> BIT_POSITION) & 0x01));
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForOutputOnPinLessThanEightTHENAMaskWithTheSpecifiedBitSetIsSent) {
    const uint8_t BIT_POSITION = 3;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    gpio_x.pinMode(3, mcp23s17::PinMode::OUTPUT);
    ASSERT_LT(2, _index);
    EXPECT_EQ(0x00, ((_spi_transaction[2] >> BIT_POSITION) & 0x01));
}

TEST_F(MockSPITransfer, pinMode$WHENCalledForOutputOnPinGreaterThanOrEqualToEightTHENAMaskWithTheSpecifiedBitSetIsSent) {
    const uint8_t BIT_POSITION = 8 % 8;
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    gpio_x.pinMode(8, mcp23s17::PinMode::OUTPUT);
    ASSERT_LT(2, _index);
    EXPECT_EQ(0x00, ((_spi_transaction[2] >> BIT_POSITION) & 0x01));
}

TEST_F(MockSPITransfer, pinMode$WHENCalledOnPinFromADifferentPortThanThePreviousCallTHENTheOriginalPinIsNotDisturbed) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
  
    gpio_x.pinMode(7, mcp23s17::PinMode::OUTPUT);
    gpio_x.pinMode(10, mcp23s17::PinMode::OUTPUT);
    EXPECT_EQ(0x00, ((gpio_x.getControlRegister()[static_cast<uint8_t>(mcp23s17::ControlRegister::IODIRA)] >> 7) & 0x01));
}

TEST_F(MockSPITransfer, pinMode$WHENCalledOnPinFromTheSamePortAsAPreviousCallTHENTheOriginalPinIsNotDisturbed) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    
    gpio_x.pinMode(8, mcp23s17::PinMode::OUTPUT);
    gpio_x.pinMode(10, mcp23s17::PinMode::OUTPUT);
    EXPECT_EQ(0x00, (gpio_x.getControlRegister()[static_cast<uint8_t>(mcp23s17::ControlRegister::IODIRB)] & 0x01));
}

TEST_F(MockSPITransfer, pinMode$WHENCalledOnPinAlreadyInTheCorrectStateTHENNoSPITransactionOccurs) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    
    // Make the original call to set the state
    gpio_x.pinMode(8, mcp23s17::PinMode::OUTPUT);
    
    // Reset the SPI transaction for next transaction
    _index = 0;
    for ( int i = 0 ; i < sizeof(_spi_transaction) ; ++ i ) { _spi_transaction[i] = 0x00; }
    MOCK::resetPinTransitions();
    
    gpio_x.pinMode(8, mcp23s17::PinMode::OUTPUT);
    EXPECT_EQ(0, _index);
}

//TODO: Test mcp23s17::PinMode::INPUT
//TODO: Handle mcp23s17::PinMode::INPUT_PULLUP

TEST_F(MockSPITransfer, digitalWrite$WHENCalledTHENTheCallersChipSelectPinIsPulledFromHighToLowAndBack) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    gpio_x.pinMode(3, mcp23s17::PinMode::OUTPUT);
    
    // Reset the SPI transaction for next transaction
    _index = 0;
    for ( int i = 0 ; i < sizeof(_spi_transaction) ; ++ i ) { _spi_transaction[i] = 0x00; }
    MOCK::resetPinTransitions();
    
    gpio_x.digitalWrite(3, mcp23s17::PinLatchValue::HIGH);
    EXPECT_EQ(MOCK::PinTransition::LOW_TO_HIGH, MOCK::getPinTransition(SS));
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledTHENAWriteTransactionIsSent) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    gpio_x.pinMode(3, mcp23s17::PinMode::OUTPUT);

    // Reset the SPI transaction for next transaction
    _index = 0;
    for ( int i = 0 ; i < sizeof(_spi_transaction) ; ++ i ) { _spi_transaction[i] = 0x00; }
    MOCK::resetPinTransitions();
    
    gpio_x.digitalWrite(3, mcp23s17::PinLatchValue::HIGH);
    ASSERT_LT(0, _index);
    EXPECT_EQ(static_cast<uint8_t>(mcp23s17::RegisterTransaction::WRITE), (_spi_transaction[0] & 0x01));
}

TEST_F(MockSPITransfer, digitalWrite$WHENCalledOnPinLessThanEightTHENTheGPIOARegisterIsTargeted) {
    TC_mcp23s17 gpio_x(mcp23s17::HardwareAddress::HW_ADDR_6);
    gpio_x.pinMode(3, mcp23s17::PinMode::OUTPUT);
    
    // Reset the SPI transaction for next transaction
    _index = 0;
    for ( int i = 0 ; i < sizeof(_spi_transaction) ; ++ i ) { _spi_transaction[i] = 0x00; }
    MOCK::resetPinTransitions();
    
    gpio_x.digitalWrite(3, mcp23s17::PinLatchValue::HIGH);
    ASSERT_LT(1, _index);
    EXPECT_EQ(static_cast<uint8_t>(mcp23s17::ControlRegister::GPIOA), _spi_transaction[1]);
}

} // namespace
/*
int main (int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
*/
/* Created and copyrighted by Zachary J. Fields. All rights reserved. */
