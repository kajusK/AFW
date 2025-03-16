#include <unity.h>
#include "modules/nmea.c"

void test_Hex2Dec(void)
{
    TEST_ASSERT_EQUAL(0, Nmeai_Hex2Dec('0'));
    TEST_ASSERT_EQUAL(3, Nmeai_Hex2Dec('3'));
    TEST_ASSERT_EQUAL(9, Nmeai_Hex2Dec('9'));
    TEST_ASSERT_EQUAL(10, Nmeai_Hex2Dec('a'));
    TEST_ASSERT_EQUAL(10, Nmeai_Hex2Dec('A'));
    TEST_ASSERT_EQUAL(15, Nmeai_Hex2Dec('F'));
}

void test_Str2Dec(void)
{
    const char *str1 = "123";
    const char *str2 = "12a";
    const char *str3 = "023";

    TEST_ASSERT_EQUAL(123, Nmeai_Str2Dec(&str1, 3));
    TEST_ASSERT_EQUAL('\0', *str1);
    TEST_ASSERT_EQUAL(12, Nmeai_Str2Dec(&str2, 3));
    TEST_ASSERT_EQUAL('a', *str2);
    TEST_ASSERT_EQUAL(2, Nmeai_Str2Dec(&str3, 2));
    TEST_ASSERT_EQUAL('3', *str3);
}

void test_Float2Decdeg(void)
{
    nmea_float_t f = { -1155892345, 100000 };

    Nmeai_Float2DecDeg(&f);
    TEST_ASSERT_EQUAL(-1159820575, f.num);
    TEST_ASSERT_EQUAL(10000000, f.scale);
}

void test_VerifyChecksum(void)
{
    TEST_ASSERT_TRUE(
        Nmea_VerifyChecksum("$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*0A"));

    TEST_ASSERT_TRUE(
        Nmea_VerifyChecksum("$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*0a"));

    TEST_ASSERT_FALSE(
        Nmea_VerifyChecksum("$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*1F"));

    TEST_ASSERT_FALSE(
        Nmea_VerifyChecksum("$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*0["));

    TEST_ASSERT_FALSE(Nmea_VerifyChecksum("GPGSA,,,,,1.38*1F"));
    TEST_ASSERT_FALSE(Nmea_VerifyChecksum("$GPGSA,,,,,1.38*1"));
    TEST_ASSERT_FALSE(Nmea_VerifyChecksum("$GPGSA,,,,,1.38*"));
    TEST_ASSERT_FALSE(Nmea_VerifyChecksum("$GPGSA,,,,,1.38"));
    TEST_ASSERT_FALSE(Nmea_VerifyChecksum(""));
    TEST_ASSERT_FALSE(Nmea_VerifyChecksum("*"));
    TEST_ASSERT_FALSE(Nmea_VerifyChecksum("$*"));
}

void test_VerifyMsg(void)
{
    TEST_ASSERT_TRUE(
        Nmea_VerifyMessage("$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*0A"));

    TEST_ASSERT_FALSE(
        Nmea_VerifyMessage("$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*1A"));

    TEST_ASSERT_FALSE(
        Nmea_VerifyMessage("$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*0"));

    TEST_ASSERT_TRUE(Nmea_VerifyMessage("$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38"));

    TEST_ASSERT_FALSE(Nmea_VerifyMessage("foo"));
    TEST_ASSERT_TRUE(Nmea_VerifyMessage("$foobar,valid"));
}

void test_Scan(void)
{
    char c;
    int8_t dir1, dir2;
    int i;
    char str[10];
    nmea_float_t f1, f2;
    nmea_date_t date;
    nmea_time_t time1, time2, time3;

    TEST_ASSERT_TRUE(
        Nmeai_Scan("GPFOO,f,ign,05,+12.04,-4912.12345,", "sc_ifp_", str, &c, &i, &f1, &f2));
    TEST_ASSERT_EQUAL_STRING("GPFOO", str);
    TEST_ASSERT_EQUAL('f', c);
    TEST_ASSERT_EQUAL(5, i);
    TEST_ASSERT_EQUAL(1204, f1.num);
    TEST_ASSERT_EQUAL(100, f1.scale);
    /* converted to decimal degrees coordinates */
    TEST_ASSERT_EQUAL(-492020575, f2.num);
    TEST_ASSERT_EQUAL(10000000, f2.scale);

    TEST_ASSERT_TRUE(Nmeai_Scan("$N,S,120125,122508,053011.123,,A*23", "DDdttt_", &dir1, &dir2,
        &date, &time1, &time2, &time3));
    TEST_ASSERT_EQUAL(1, dir1);
    TEST_ASSERT_EQUAL(-1, dir2);
    TEST_ASSERT_EQUAL(12, date.day);
    TEST_ASSERT_EQUAL(1, date.month);
    TEST_ASSERT_EQUAL(25, date.year);
    TEST_ASSERT_EQUAL(12, time1.hour);
    TEST_ASSERT_EQUAL(25, time1.minute);
    TEST_ASSERT_EQUAL(8, time1.second);
    TEST_ASSERT_EQUAL(0, time1.micros);
    TEST_ASSERT_EQUAL(5, time2.hour);
    TEST_ASSERT_EQUAL(30, time2.minute);
    TEST_ASSERT_EQUAL(11, time2.second);
    TEST_ASSERT_EQUAL(123000, time2.micros);
    TEST_ASSERT_EQUAL(-1, time3.hour);
    TEST_ASSERT_EQUAL(-1, time3.minute);
    TEST_ASSERT_EQUAL(-1, time3.second);
    TEST_ASSERT_EQUAL(0, time3.micros);

    TEST_ASSERT_FALSE(Nmeai_Scan("N,S", "DDdtt", &dir1, &dir2, &date, &time1, &time2));
    TEST_ASSERT_FALSE(Nmeai_Scan("N,S", "D", &dir1));
}

void test_ParseRmc(void)
{
    nmea_rmc_t rmc;

    TEST_ASSERT_FALSE(
        Nmea_ParseRmc("$GPFOO,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E", &rmc));

    TEST_ASSERT_TRUE(
        Nmea_ParseRmc("$GPRMC,081836,A,3751.65,S,14507.36,E,999.99,123.4,130998,011.3,W", &rmc));

    TEST_ASSERT_TRUE(rmc.valid);
    TEST_ASSERT_EQUAL(8, rmc.fix_time.hour);
    TEST_ASSERT_EQUAL(18, rmc.fix_time.minute);
    TEST_ASSERT_EQUAL(36, rmc.fix_time.second);
    TEST_ASSERT_EQUAL(-378608, rmc.lat.num);
    TEST_ASSERT_EQUAL(10000, rmc.lat.scale);
    TEST_ASSERT_EQUAL(1451226, rmc.lon.num);
    TEST_ASSERT_EQUAL(10000, rmc.lon.scale);
    /* 999,99 knots is equal to 514.43929999556 m/s */
    TEST_ASSERT_EQUAL(51439, rmc.speed_ms.num);
    TEST_ASSERT_EQUAL(100, rmc.speed_ms.scale);
    TEST_ASSERT_EQUAL(1234, rmc.heading.num);
    TEST_ASSERT_EQUAL(10, rmc.heading.scale);
    TEST_ASSERT_EQUAL(13, rmc.date.day);
    TEST_ASSERT_EQUAL(9, rmc.date.month);
    TEST_ASSERT_EQUAL(98, rmc.date.year);
    TEST_ASSERT_EQUAL(-113, rmc.mag_variation.num);
    TEST_ASSERT_EQUAL(10, rmc.mag_variation.scale);

    TEST_ASSERT_TRUE(
        Nmea_ParseRmc("$GPRMC,191118.000,A,4911.3987,N,01745.4449,E,12.561,6.42,241020,,,A", &rmc));
    TEST_ASSERT_TRUE(rmc.valid);
    TEST_ASSERT_EQUAL(19, rmc.fix_time.hour);
    TEST_ASSERT_EQUAL(11, rmc.fix_time.minute);
    TEST_ASSERT_EQUAL(18, rmc.fix_time.second);
    TEST_ASSERT_EQUAL(49189978, rmc.lat.num);
    TEST_ASSERT_EQUAL(1000000, rmc.lat.scale);
    TEST_ASSERT_EQUAL(17757415, rmc.lon.num);
    TEST_ASSERT_EQUAL(1000000, rmc.lon.scale);
    TEST_ASSERT_EQUAL(646, rmc.speed_ms.num);
    TEST_ASSERT_EQUAL(100, rmc.speed_ms.scale);
    TEST_ASSERT_EQUAL(642, rmc.heading.num);
    TEST_ASSERT_EQUAL(100, rmc.heading.scale);
    TEST_ASSERT_EQUAL(24, rmc.date.day);
    TEST_ASSERT_EQUAL(10, rmc.date.month);
    TEST_ASSERT_EQUAL(20, rmc.date.year);
    TEST_ASSERT_EQUAL(0, rmc.mag_variation.num);
    TEST_ASSERT_EQUAL(1, rmc.mag_variation.scale);

    /* Used on L96 and similar */
    TEST_ASSERT_TRUE(
        Nmea_ParseRmc("$GNRMC,181320.000,A,4238.4047,N,01141.4529,E,0.00,356.03,040621,,,A,V",
            &rmc));
}

void test_ParseGga(void)
{
    nmea_gga_t gga;

    TEST_ASSERT_FALSE(
        Nmea_ParseGga("$GPFOO,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,", &gga));

    TEST_ASSERT_TRUE(
        Nmea_ParseGga("$GPGGA,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,*76",
            &gga));
    TEST_ASSERT_EQUAL(9, gga.fix_time.hour);
    TEST_ASSERT_EQUAL(27, gga.fix_time.minute);
    TEST_ASSERT_EQUAL(50, gga.fix_time.second);
    TEST_ASSERT_EQUAL(53361336, gga.lat.num);
    TEST_ASSERT_EQUAL(1000000, gga.lat.scale);
    TEST_ASSERT_EQUAL(-6505620, gga.lon.num);
    TEST_ASSERT_EQUAL(1000000, gga.lon.scale);
    TEST_ASSERT_EQUAL(1, gga.quality);
    TEST_ASSERT_EQUAL(8, gga.satellites);
    TEST_ASSERT_EQUAL(103, gga.hdop.num);
    TEST_ASSERT_EQUAL(100, gga.hdop.scale);
    TEST_ASSERT_EQUAL(617, gga.altitude_m.num);
    TEST_ASSERT_EQUAL(10, gga.altitude_m.scale);
    TEST_ASSERT_EQUAL(552, gga.above_ellipsoid_m.num);
    TEST_ASSERT_EQUAL(10, gga.above_ellipsoid_m.scale);
}

void test_ParseGsv(void)
{
    nmea_gsv_t gsv;

    TEST_ASSERT_FALSE(
        Nmea_ParseGsv("$GPGSF,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D", &gsv));

    TEST_ASSERT_TRUE(
        Nmea_ParseGsv("$GPGSV,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D", &gsv));

    TEST_ASSERT_EQUAL(3, gsv.messages);
    TEST_ASSERT_EQUAL(3, gsv.msg_id);
    TEST_ASSERT_EQUAL(11, gsv.visible);
    TEST_ASSERT_EQUAL(3, gsv.count);

    TEST_ASSERT_EQUAL(22, gsv.sv[0].prn);
    TEST_ASSERT_EQUAL(42, gsv.sv[0].elevation);
    TEST_ASSERT_EQUAL(67, gsv.sv[0].azimuth);
    TEST_ASSERT_EQUAL(42, gsv.sv[0].snr);

    TEST_ASSERT_EQUAL(24, gsv.sv[1].prn);
    TEST_ASSERT_EQUAL(14, gsv.sv[1].elevation);
    TEST_ASSERT_EQUAL(311, gsv.sv[1].azimuth);
    TEST_ASSERT_EQUAL(43, gsv.sv[1].snr);

    TEST_ASSERT_EQUAL(27, gsv.sv[2].prn);
    TEST_ASSERT_EQUAL(5, gsv.sv[2].elevation);
    TEST_ASSERT_EQUAL(244, gsv.sv[2].azimuth);
    TEST_ASSERT_EQUAL(0, gsv.sv[2].snr);
}

void test_GetSentenceType(void)
{
    TEST_ASSERT_EQUAL(NMEA_SENTENCE_RMC,
        Nmea_GetSentenceType("$GPRMC,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*62"));

    TEST_ASSERT_EQUAL(NMEA_SENTENCE_GGA,
        Nmea_GetSentenceType(
            "$GPGGA,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,*76"));

    TEST_ASSERT_EQUAL(NMEA_SENTENCE_GSV,
        Nmea_GetSentenceType("$GPGSV,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D"));

    TEST_ASSERT_EQUAL(NMEA_SENTENCE_UNKNOWN,
        Nmea_GetSentenceType(
            "$GPFOO,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,"));
}

void test_AddChar(void)
{
    char buf[] = "$foobar,444,123,112123,232321,*32";
    char buf2[] = "$foobar,444,123,*32";
    const char *res;

    for (size_t i = 0; i < strlen(buf); i++) {
        res = Nmea_AddChar(buf[i]);
        TEST_ASSERT_NULL(res);
    }
    res = Nmea_AddChar('\n');
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_STRING(buf, res);

    for (size_t i = 0; i < strlen(buf2); i++) {
        res = Nmea_AddChar(buf2[i]);
        TEST_ASSERT_NULL(res);
    }
    res = Nmea_AddChar('\n');
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_STRING(buf2, res);
}
