/*
    Copyright 2025 (C) Alexey Dynda

    This file is part of Tiny Protocol Library.

    GNU General Public License Usage

    Protocol Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Protocol Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Protocol Library.  If not, see <http://www.gnu.org/licenses/>.

    Commercial License Usage

    Licensees holding valid commercial Tiny Protocol licenses may use this file in
    accordance with the commercial license agreement provided in accordance with
    the terms contained in a written agreement between you and Alexey Dynda.
    For further information contact via email on github account.

*/

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