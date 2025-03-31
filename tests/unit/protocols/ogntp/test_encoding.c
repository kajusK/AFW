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

void test_decodeFromUint14(void)
{
    TEST_ASSERT_EQUAL_HEX16(0x0000, decodeFromUint14(0));
    TEST_ASSERT_EQUAL_HEX16(0x1001, decodeFromUint14(0x1000));
    TEST_ASSERT_EQUAL_HEX16(0x1469, decodeFromUint14(0x1234));
    TEST_ASSERT_EQUAL_HEX16(0x6ffe, decodeFromUint14(0x2fff));
    TEST_ASSERT_EQUAL_HEX16(0x7004, decodeFromUint14(0x3000));
    TEST_ASSERT_EQUAL_HEX16(0x791c, decodeFromUint14(0x3123));
    TEST_ASSERT_EQUAL_HEX16(0xeffc, decodeFromUint14(0x3fff));
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

void test_decodeFromUint10(void)
{
    TEST_ASSERT_EQUAL_HEX16(0x0000, decodeFromUint10(0));
    TEST_ASSERT_EQUAL_HEX16(0x0101, decodeFromUint10(0x100));
    TEST_ASSERT_EQUAL_HEX16(0x0147, decodeFromUint10(0x123));
    TEST_ASSERT_EQUAL_HEX16(0x06fe, decodeFromUint10(0x2ff));
    TEST_ASSERT_EQUAL_HEX16(0x0704, decodeFromUint10(0x300));
    TEST_ASSERT_EQUAL_HEX16(0x0794, decodeFromUint10(0x312));
    TEST_ASSERT_EQUAL_HEX16(0x0efc, decodeFromUint10(0x3ff));
}

void test_encodeToUint6(void)
{
    TEST_ASSERT_EQUAL_HEX8(0x00, encodeToUint6(0));
    TEST_ASSERT_EQUAL_HEX8(0x0f, encodeToUint6(0x10 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x10, encodeToUint6(0x10));
    TEST_ASSERT_EQUAL_HEX8(0x1f, encodeToUint6(0x30 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x20, encodeToUint6(0x30));
    TEST_ASSERT_EQUAL_HEX8(0x2f, encodeToUint6(0x70 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x30, encodeToUint6(0x70));
    TEST_ASSERT_EQUAL_HEX8(0x3f, encodeToUint6(0xf0 - 1));
    TEST_ASSERT_EQUAL_HEX8(0x3f, encodeToUint6(0xf0));
    TEST_ASSERT_EQUAL_HEX8(0x3f, encodeToUint6(0xff));
}

void test_decodeFromUint6(void)
{
    TEST_ASSERT_EQUAL_HEX16(0x00, decodeFromUint6(0));
    TEST_ASSERT_EQUAL_HEX16(0x11, decodeFromUint6(0x10));
    TEST_ASSERT_EQUAL_HEX16(0x15, decodeFromUint6(0x12));
    TEST_ASSERT_EQUAL_HEX16(0x6e, decodeFromUint6(0x2f));
    TEST_ASSERT_EQUAL_HEX16(0x74, decodeFromUint6(0x30));
    TEST_ASSERT_EQUAL_HEX16(0x7c, decodeFromUint6(0x31));
    TEST_ASSERT_EQUAL_HEX16(0xec, decodeFromUint6(0x3f));
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

void test_decodeSignedFromUint8(void)
{
    TEST_ASSERT_EQUAL_HEX8(0, decodeSignedFromUint8(0x00));
    TEST_ASSERT_EQUAL_HEX8(31, decodeSignedFromUint8(0x1f));
    TEST_ASSERT_EQUAL_HEX8(33, decodeSignedFromUint8(0x20));
    TEST_ASSERT_EQUAL_HEX8(95, decodeSignedFromUint8(0x3f));
    TEST_ASSERT_EQUAL_HEX8(98, decodeSignedFromUint8(0x40));
    TEST_ASSERT_EQUAL_HEX8(222, decodeSignedFromUint8(0x5f));
    TEST_ASSERT_EQUAL_HEX8(228, decodeSignedFromUint8(0x60));
    TEST_ASSERT_EQUAL_HEX8(476, decodeSignedFromUint8(0x7f));

    TEST_ASSERT_EQUAL_HEX8(0, decodeSignedFromUint8(0x80));
    TEST_ASSERT_EQUAL_HEX8(-6, decodeSignedFromUint8(0x86));
    TEST_ASSERT_EQUAL_HEX8(-33, decodeSignedFromUint8(0xa0));
    TEST_ASSERT_EQUAL_HEX8(-47, decodeSignedFromUint8(0xa7));
    TEST_ASSERT_EQUAL_HEX8(-98, decodeSignedFromUint8(0xc0));
    TEST_ASSERT_EQUAL_HEX8(-122, decodeSignedFromUint8(0xc6));
    TEST_ASSERT_EQUAL_HEX8(-228, decodeSignedFromUint8(0xe0));
    TEST_ASSERT_EQUAL_HEX8(-476, decodeSignedFromUint8(0xff));
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

void test_decodeSignedFromUint9(void)
{
    TEST_ASSERT_EQUAL_HEX8(0, decodeSignedFromUint9(0x00));
    TEST_ASSERT_EQUAL_HEX8(63, decodeSignedFromUint9(0x3f));
    TEST_ASSERT_EQUAL_HEX8(73, decodeSignedFromUint9(0x44));
    TEST_ASSERT_EQUAL_HEX8(191, decodeSignedFromUint9(0x7f));
    TEST_ASSERT_EQUAL_HEX8(226, decodeSignedFromUint9(0x88));
    TEST_ASSERT_EQUAL_HEX8(446, decodeSignedFromUint9(0xbf));
    TEST_ASSERT_EQUAL_HEX8(476, decodeSignedFromUint9(0xc3));
    TEST_ASSERT_EQUAL_HEX8(956, decodeSignedFromUint9(0xff));

    TEST_ASSERT_EQUAL_HEX8(0, decodeSignedFromUint9(0x100));
    TEST_ASSERT_EQUAL_HEX8(-63, decodeSignedFromUint9(0x13f));
    TEST_ASSERT_EQUAL_HEX8(-73, decodeSignedFromUint9(0x144));
    TEST_ASSERT_EQUAL_HEX8(-191, decodeSignedFromUint9(0x17f));
    TEST_ASSERT_EQUAL_HEX8(-226, decodeSignedFromUint9(0x188));
    TEST_ASSERT_EQUAL_HEX8(-446, decodeSignedFromUint9(0x1bf));
    TEST_ASSERT_EQUAL_HEX8(-476, decodeSignedFromUint9(0x1c3));
    TEST_ASSERT_EQUAL_HEX8(-956, decodeSignedFromUint9(0x1ff));
}
