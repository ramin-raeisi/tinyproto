#include <CppUTest/TestHarness.h>
#include "proto/crc/tiny_crc.h"

TEST_GROUP(TinyCrcTests)
{
    void setup() {}
    void teardown() {}
};

TEST(TinyCrcTests, ChkSum)
{
#ifdef CONFIG_ENABLE_CHECKSUM
    uint8_t buf[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint16_t result = tiny_chksum(INITCHECKSUM, buf, sizeof(buf));
    // Verify a known sum
    CHECK_EQUAL((uint16_t)(0xFFFF - (1+2+3+4+5+6+7+8)), result);
#endif
}

TEST(TinyCrcTests, Crc16)
{
#ifdef CONFIG_ENABLE_FCS16
    uint8_t buf[4] = {0x12, 0x34, 0xAB, 0xCD};
    uint16_t crc = tiny_crc16(PPPINITFCS16, buf, sizeof(buf));
    // Just check it doesn't match the initial CRC
    CHECK_TRUE(crc != PPPINITFCS16);
#endif
}

TEST(TinyCrcTests, Crc32)
{
#ifdef CONFIG_ENABLE_FCS32
    uint8_t buf[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint32_t crc = tiny_crc32(PPPINITFCS32, buf, sizeof(buf));
    // Just check it doesn't match the initial CRC
    CHECK_TRUE(crc != PPPINITFCS32);
#endif
}