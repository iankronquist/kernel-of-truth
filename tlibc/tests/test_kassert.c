#define TEST
#if !defined(__linux__)
	#error "this test should be run on linux"
#endif

#include "../tstdlib.h"
#include <stdio.h>

void test_assert_should_succeed();
void test_assert_should_fail();

int main()
{
	test_assert_should_succeed();
	test_assert_should_fail();
	return 0;
}

void test_assert_should_succeed()
{
	assert(1 == 1);
	assert(1 & 1);
	assert(1 || 0);
}

void test_assert_should_fail()
{
	assert(0 | 0);
}

