#include <unity.h>
#include "utils/random.c"

void test_randomGenerator(void)
{
    Random_Init(0xabcdef12);
    TEST_ASSERT_EQUAL_HEX32(0xD3DB24A5, Random_Get());
    TEST_ASSERT_EQUAL_HEX32(0x5EB43F42, Random_Get());
    TEST_ASSERT_EQUAL_HEX32(0xF2DE6E6C, Random_Get());
    TEST_ASSERT_EQUAL_HEX32(0xDD6DCD45, Random_Get());

    Random_Init(0x5EB43F42);
    TEST_ASSERT_EQUAL_HEX32(0xF2DE6E6C, Random_Get());

    Random_Init(0xabcdef12);
    TEST_ASSERT_EQUAL_HEX32(0xD3DB24A5, Random_Get());
}
