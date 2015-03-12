/* Created and copyrighted by Zachary J. Fields. All rights reserved. */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../mcp23s17.h"

/*
template <typename return_code_t>
class TC_Untitled : public TC_Untitled<return_code_t> {
  public:
	TC_Untitled (
		unsigned char i_parameter_
	): Untitled<return_code_t>(i_parameter_)
	{}
	
	// Access protected test members
	using Untitled<return_code_t>::STATIC_CONST_MEMBER_VARIABLE;
	using Untitled<return_code_t>::_member_variable;	
	using Untitled<return_code_t>::memberMethod;
};
*/
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
	mcp23s17 gpio_x(mcp23s17::HW_ADDR_6);
	ASSERT_EQ(mcp23s17::HW_ADDR_6, gpio_x.getHardwareAddress());
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
