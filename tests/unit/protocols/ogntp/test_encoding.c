#include <unity.h>
#include "protocols/ogntp/encoding.c"

void test_encodeToUint14(void)
{
    TEST_ASSERT_EQUAL_HEX16(0x0000, encodeToUint14(0));
    TEST_ASSERT_EQUAL_HEX16(0x0fff, encodeToUint14(0x1000 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x1000, encodeToUint14(0x1000));
    TEST_ASSERT_EQUAL_HEX16(0x2fff, encodeToUint14(0x7000 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x3000, encodeToUint14(0x7000));
    TEST_ASSERT_EQUAL_HEX16(0x3fff, encodeToUint14(0xf000 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x3fff, encodeToUint14(0xf000));
    TEST_ASSERT_EQUAL_HEX16(0x3fff, encodeToUint14(0xffff));
}

void test_encodeToUint10(void)
{
    TEST_ASSERT_EQUAL_HEX16(0x0000, encodeToUint10(0));
    TEST_ASSERT_EQUAL_HEX16(0x00ff, encodeToUint10(0x100 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x0100, encodeToUint10(0x100));
    TEST_ASSERT_EQUAL_HEX16(0x01ff, encodeToUint10(0x300 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x0200, encodeToUint10(0x300));
    TEST_ASSERT_EQUAL_HEX16(0x02ff, encodeToUint10(0x700 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x0300, encodeToUint10(0x700));
    TEST_ASSERT_EQUAL_HEX16(0x03ff, encodeToUint10(0xf00 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x03ff, encodeToUint10(0xf00));
    TEST_ASSERT_EQUAL_HEX16(0x03ff, encodeToUint10(0xfff));
}

void test_encodeToUint4(void)
{
    TEST_ASSERT_EQUAL_HEX8(0x00, encodeToUint4(0));
    TEST_ASSERT_EQUAL_HEX8(0x0f, encodeToUint4(0x10 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x10, encodeToUint4(0x10));
    TEST_ASSERT_EQUAL_HEX8(0x1f, encodeToUint4(0x30 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x20, encodeToUint4(0x30));
    TEST_ASSERT_EQUAL_HEX8(0x2f, encodeToUint4(0x70 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x30, encodeToUint4(0x70));
    TEST_ASSERT_EQUAL_HEX8(0x3f, encodeToUint4(0xf0 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x3f, encodeToUint4(0xf0));
    TEST_ASSERT_EQUAL_HEX8(0x3f, encodeToUint4(0xff));
}

void test_encodeSignedToUint8(void)
{
    TEST_ASSERT_EQUAL_HEX8(0x00, encodeSignedToUint8(0));
    TEST_ASSERT_EQUAL_HEX8(0x1f, encodeSignedToUint8(0x020 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x20, encodeSignedToUint8(0x020));
    TEST_ASSERT_EQUAL_HEX8(0x3f, encodeSignedToUint8(0x060 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x40, encodeSignedToUint8(0x060));
    TEST_ASSERT_EQUAL_HEX8(0x5f, encodeSignedToUint8(0x0e0 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x60, encodeSignedToUint8(0x0e0));
    TEST_ASSERT_EQUAL_HEX8(0x7f, encodeSignedToUint8(0x1e0 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x7f, encodeSignedToUint8(0x1e0));
    TEST_ASSERT_EQUAL_HEX8(0x7f, encodeSignedToUint8(0xfff));

    TEST_ASSERT_EQUAL_HEX8(0xa0, encodeSignedToUint8(-0x020 - 1));
    TEST_ASSERT_EQUAL_HEX8(0xa0, encodeSignedToUint8(-0x020));
    TEST_ASSERT_EQUAL_HEX8(0xc0, encodeSignedToUint8(-0x060 - 1));
    TEST_ASSERT_EQUAL_HEX8(0xc0, encodeSignedToUint8(-0x060));
    TEST_ASSERT_EQUAL_HEX8(0xe0, encodeSignedToUint8(-0x0e0 - 1));
    TEST_ASSERT_EQUAL_HEX8(0xe0, encodeSignedToUint8(-0x0e0));
    TEST_ASSERT_EQUAL_HEX8(0xff, encodeSignedToUint8(-0x1e0 - 1));
    TEST_ASSERT_EQUAL_HEX8(0xff, encodeSignedToUint8(-0x1e0));
    TEST_ASSERT_EQUAL_HEX8(0xff, encodeSignedToUint8(-0xfff));
}

void test_encodeSignedToUint9(void)
{
    TEST_ASSERT_EQUAL_HEX16(0x0000, encodeSignedToUint9(0));
    TEST_ASSERT_EQUAL_HEX16(0x003f, encodeSignedToUint9(0x040 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x0040, encodeSignedToUint9(0x040));
    TEST_ASSERT_EQUAL_HEX16(0x007f, encodeSignedToUint9(0x0c0 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x0080, encodeSignedToUint9(0x0c0));
    TEST_ASSERT_EQUAL_HEX16(0x00bf, encodeSignedToUint9(0x1c0 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x00c0, encodeSignedToUint9(0x1c0));
    TEST_ASSERT_EQUAL_HEX16(0x00ff, encodeSignedToUint9(0x3c0 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x00ff, encodeSignedToUint9(0x3c0));
    TEST_ASSERT_EQUAL_HEX16(0x00ff, encodeSignedToUint9(0xfff));

    TEST_ASSERT_EQUAL_HEX16(0x0140, encodeSignedToUint9(-0x040 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x0140, encodeSignedToUint9(-0x040));
    TEST_ASSERT_EQUAL_HEX16(0x0180, encodeSignedToUint9(-0x0c0 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x0180, encodeSignedToUint9(-0x0c0));
    TEST_ASSERT_EQUAL_HEX16(0x01c0, encodeSignedToUint9(-0x1c0 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x01c0, encodeSignedToUint9(-0x1c0));
    TEST_ASSERT_EQUAL_HEX16(0x01ff, encodeSignedToUint9(-0x3c0 - 1));
    TEST_ASSERT_EQUAL_HEX16(0x01ff, encodeSignedToUint9(-0x3c0));
    TEST_ASSERT_EQUAL_HEX16(0x01ff, encodeSignedToUint9(-0xfff));
}
