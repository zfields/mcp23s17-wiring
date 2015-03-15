/* Created and copyrighted by Zachary J. Fields. All rights reserved. */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../mcp23s17.h"
#include "MOCK_spi.h"

class TC_mcp23s17 : public mcp23s17 {
  public:
	TC_mcp23s17 (
		mcp23s17::HardwareAddress hw_addr_
	): mcp23s17(hw_addr_)
	{}
	
	// Access protected test members
	using mcp23s17::getControlRegisterAddresses;
};

namespace {
/*
class ScenarioFixture : public ::testing::Test {
  protected:
	TC_Untitled<return_code_t> *tc_untitled;
	
	ScenarioFixture() {
		// This happens before SetUp()
	}
	~ScenarioFixture() {
		// This happens after TearDown()
	}
	
	void SetUp (void) {
		tc_untitled = new TC_Untitled<return_code_t>(7);
	}
	void TearDown (void) {
		delete(tc_untitled);
	}
};
*/
/*
The first argument is the name of the test case, and the second argument
is the test's name within the test case. Both names must be valid C++
identifiers, and they should not contain underscore (_). A test's full
name consists of its containing test case and its individual name. Tests
from different test cases can have the same individual name.
(e.g. ASSERT_EQ(_EXPECTED_, _ACTUAL_))
*/
TEST(Construction, WHENObjectIsConstructedTHENAddressParameterIsStored) {
	TC_mcp23s17 gpio_x(mcp23s17::HW_ADDR_6);
	ASSERT_EQ(0x4C, gpio_x.getSpiBusAddress());
}

TEST(Construction, WHENObjectIsConstructedTHENControlRegisterAddressesArePopulatedWithBankEqualsZeroValues) {
	TC_mcp23s17 gpio_x(mcp23s17::HW_ADDR_6);
	for ( int i = 0 ; i < mcp23s17::REGISTER_COUNT ; ++i ) {
		EXPECT_EQ(i, gpio_x.getControlRegisterAddresses()[i]) << "Expected value <" << i << ">!";
	}
}

/*
Like TEST(), the first argument is the test case name, but for TEST_F()
this must be the name of the test fixture class.
*/
/*
TEST_F(ScenarioFixture, TestName) {
	const int EXPECTED_VALUE(7);
	int actual_value(6);
	
	EXPECT_EQ(EXPECTED_VALUE, actual_value);
	EXPECT_TRUE(false);
	ASSERT_TRUE(true);
}
*/
} // namespace
/*
int main (int argc, char *argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
*/
/* Created and copyrighted by Zachary J. Fields. All rights reserved. */
