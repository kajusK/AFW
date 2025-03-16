#include <unity.h>
#include "protocols/ogntp/encoding.c"
#include "protocols/ogntp/fcs.c"
#include "protocols/ogntp/whitening.c"
#include "utils/utils.c"
#include "protocols/ogntp/ogntp.c"

void test_EncodePosition(void)
{
    uint32_t buffer[OGNTP_FRAME_BYTES];
    const uint32_t expected[] = { 0x03ddeeff, 0x86c383e2, 0xbb5e931d, 0xca3d88c8, 0x7cabf3f1,
        0xa54773e5, 0x00007aa8 };
    ogntp_aircraft_t aircraft = {
        .address = 0xddeeff,
        .addr_type = OGNTP_ADDRESS_OGN,
        .type = OGNTP_AIRCRAFT_POWERED,
    };
    gps_info_t gps = {
        .latitude.num = 491951,
        .latitude.scale = 10000,
        .longitude.num = 166068,
        .longitude.scale = 10000,
        .altitude_dm = 1236,  // 123.6 m
        .speed_dms = 128,     // 12.8 m/s
        .heading_ddeg = 3422, // 342.2 degrees
        .hdop_d = 128,        // 12.8
        .is_3d_fix = true,
        .fix_quality = GPS_FIX_DGPS,
        .time.second = 52,
    };

    OGNTP_EncodePosition((uint8_t *)buffer, &aircraft, &gps);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buffer, OGNTP_FRAME_BYTES);
}
