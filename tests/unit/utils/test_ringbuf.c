#include <unity.h>
#include <string.h>
#include "utils/ringbuf.c"

static char buf[4];
static ring_t rbuf;

void setUp(void)
{
    memset(buf, 0, 4);
    Ring_Init(&rbuf, buf, 4);
}

void test_ringbuf(void)
{
    TEST_ASSERT_TRUE(Ring_Empty(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));

    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'a'));
    TEST_ASSERT_FALSE(Ring_Empty(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));

    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'b'));
    TEST_ASSERT_FALSE(Ring_Empty(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));

    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'c'));
    TEST_ASSERT_FALSE(Ring_Empty(&rbuf));
    TEST_ASSERT_TRUE(Ring_Full(&rbuf));

    TEST_ASSERT_FALSE(Ring_Push(&rbuf, 'd'));
    TEST_ASSERT_FALSE(Ring_Empty(&rbuf));
    TEST_ASSERT_TRUE(Ring_Full(&rbuf));

    TEST_ASSERT_EQUAL('a', Ring_Pop(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));

    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'e'));
    TEST_ASSERT_TRUE(Ring_Full(&rbuf));

    TEST_ASSERT_EQUAL('b', Ring_Pop(&rbuf));
    TEST_ASSERT_EQUAL('c', Ring_Pop(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));
    TEST_ASSERT_FALSE(Ring_Empty(&rbuf));

    TEST_ASSERT_EQUAL('e', Ring_Pop(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));
    TEST_ASSERT_TRUE(Ring_Empty(&rbuf));

    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'f'));
    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'g'));
    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'h'));
    TEST_ASSERT_TRUE(Ring_Full(&rbuf));
    TEST_ASSERT_EQUAL('f', Ring_Pop(&rbuf));
    TEST_ASSERT_EQUAL('g', Ring_Pop(&rbuf));
    TEST_ASSERT_EQUAL('h', Ring_Pop(&rbuf));
    TEST_ASSERT_TRUE(Ring_Empty(&rbuf));

    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'f'));
    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'g'));
    Ring_Clear(&rbuf);
    TEST_ASSERT_TRUE(Ring_Empty(&rbuf));
    TEST_ASSERT_EQUAL(-1, Ring_Pop(&rbuf));
}
