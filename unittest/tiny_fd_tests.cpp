/*
    Copyright 2019-2024 (C) Alexey Dynda

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

#include <functional>
#include <CppUTest/TestHarness.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <array>

#include "proto/fd/tiny_fd.h"

TEST_GROUP(TINY_FD)
{
    void setup()
    {
        connected = false;
        tiny_fd_init_t init{};
        init.pdata = this;
        init.on_connect_event_cb = __onConnect;
        init.on_read_cb = onRead;
        init.buffer = inBuffer.data();
        init.buffer_size = inBuffer.size();
        init.window_frames = 7;
        init.send_timeout = 1000;
        init.retry_timeout = 100;
        init.retries = 2;
        init.mode = TINY_FD_MODE_ABM;
        init.peers_count = 1; // For ABM mode, only one peer is needed
        init.crc_type = HDLC_CRC_OFF;
        auto result = tiny_fd_init(&handle, &init);
        CHECK_EQUAL(TINY_SUCCESS, result);
    }

    void teardown()
    {
        tiny_fd_close(handle);
    }

    void onConnect(uint8_t, bool status) { connected = status; }
    void onRead(uint8_t, uint8_t *, int) { }

    static void __onConnect(void *handle, uint8_t address, bool connected)
    {
        // get the instance of the test class
        auto *self = static_cast<TEST_GROUP_CppUTestGroupTINY_FD *>(handle);
        self->onConnect(address, connected);
    }

    static void onRead(void *handle, uint8_t address, uint8_t *buf, int len)
    {
        // get the instance of the test class
        auto *self = static_cast<TEST_GROUP_CppUTestGroupTINY_FD *>(handle);
        self->onRead(address, buf, len);
    }

    tiny_fd_handle_t handle = nullptr;
    bool connected = false;
    std::array<uint8_t, 1024> inBuffer{};
    std::array<uint8_t, 1024> outBuffer{};

    void establishConnection()
    {
        // For command requests CR bit must be set, that is why address is 0x03
        auto read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x03\x2F\x7E", 4); // SABM frame
        CHECK_EQUAL(TINY_SUCCESS, read_result);
        auto len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
        CHECK(connected); // Connection should be established
        CHECK_EQUAL(4, len);
    }
};


TEST(TINY_FD, ABM_ConnectDisconnectResponse)
{
    // For command requests CR bit must be set, that is why address is 0x03
    auto read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x03\x2F\x7E", 4); // SABM frame
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    int len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(4, len);
    // Check UA frame
    // UA frame should be: 0x7E 0x03 0x00 0x7E
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x01, outBuffer[1]); // Address field
    CHECK_EQUAL(0x73, outBuffer[2]); // UA packet
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
    CHECK(connected); // Connection should be established
    read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x03\x43\x7E", 4); // DISC frame
    CHECK_EQUAL(TINY_SUCCESS, read_result); // Should read 7 bytes
    len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(4, len);
    // Check UA frame
    // UA frame should be: 0x7E 0x03 0x00 0x7E
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x01, outBuffer[1]); // Address field - CR bit must be cleared
    CHECK_EQUAL(0x73, outBuffer[2]); // UA packet
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
    CHECK(!connected); // Connection should be disconnected 
}

TEST(TINY_FD, ABM_ConnecionTestRequest)
{
    // TODO 
}

TEST(TINY_FD, ABM_DisconnectResponseWhenNotConnected)
{
    connected = true; // this flag should not be changed if not connected
    auto read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x03\x43\x7E", 4); // DISC frame
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    int len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(4, len);
    // Check UA frame   
    // UA frame should be: 0x7E 0x03 0x00 0x7E
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x01, outBuffer[1]); // address field - CR bit must be cleared
    CHECK_EQUAL(0x73, outBuffer[2]); // UA packet
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
    CHECK(connected); // Connection should not be changed
}

TEST(TINY_FD, ABM_RecieveTwoConsequentIFrames)
{
    establishConnection();
    // Now we can send I-frames
    auto read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x03\x00\x11\x7E", 5); // I-frame with data
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x03\x02\x22\x7E", 5); // I-frame with data
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    auto len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(8, len);
    // Check RR frame
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x01, outBuffer[1]); // Address field - CR bit must be cleared
    CHECK_EQUAL(0x31, outBuffer[2]); // RR packet with N(R) = 1
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
    // Check RR frame
    CHECK_EQUAL(0x7E, outBuffer[4]); // Flag
    CHECK_EQUAL(0x01, outBuffer[5]); // Address field - CR bit must be cleared
    CHECK_EQUAL(0x51, outBuffer[6]); // RR packet with N(R) = 2
    CHECK_EQUAL(0x7E, outBuffer[7]); // Flag
    // Now we can send I-frames again
}

TEST(TINY_FD, ABM_RecieveOutOfOrderIFrames)
{
    establishConnection();
    // Now we can send I-frames
    auto read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x03\x00\x11\x7E", 5); // I-frame in order
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x03\x04\x22\x7E", 5); // I-frame out of order
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    auto len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(8, len);
    // Check RR frame
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x01, outBuffer[1]); // Address field - CR bit must be cleared
    CHECK_EQUAL(0x31, outBuffer[2]); // RR packet with N(R) = 1
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
    // Check RR frame
    CHECK_EQUAL(0x7E, outBuffer[4]); // Flag
    CHECK_EQUAL(0x03, outBuffer[5]); // Address field - CR bit must be cleared
    CHECK_EQUAL(0x39, outBuffer[6]); // REJ packet with N(R) = 2
    CHECK_EQUAL(0x7E, outBuffer[7]); // Flag
}

TEST(TINY_FD, ABM_SendSABMOnIFrameIfDisconnected)
{
    // If we are disconnected, we should send SABM frame on I-frame
    auto read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x03\x00\x11\x7E", 5); // I-frame in order
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    auto len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(4, len);
    // Check SABM frame
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x03, outBuffer[1]); // Address field - CR bit must be cleared
    CHECK_EQUAL(0x3F, outBuffer[2]); // SABM packet
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
}

TEST(TINY_FD, ABM_RunRxAPIVerification)
{
    // For command requests CR bit must be set, that is why address is 0x03
    auto read_result = tiny_fd_run_rx(handle, [](void *user_data, void *buf, int len) -> int {
        // Simulate reading data from a source
        memcpy(buf, "\x7E\x03\x2F\x7E", 4); // SABM frame
        return 4; // Return number of bytes read
    });
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    int len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(4, len);
    // Check UA frame
    // UA frame should be: 0x7E 0x03 0x00 0x7E
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x01, outBuffer[1]); // Address field
    CHECK_EQUAL(0x73, outBuffer[2]); // UA packet
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
}

TEST(TINY_FD, ABM_RunTxAPIVerification)
{
    // For command requests CR bit must be set, that is why address is 0x03
    auto read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x03\x2F\x7E", 4); // SABM frame
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    int len = tiny_fd_run_tx(handle, [](void *user_data, const void *buf, int len) -> int {
        CHECK_EQUAL(4, len); // Expecting to write 4 bytes
        CHECK_EQUAL(0x7E, ((const uint8_t *)buf)[0]); // Flag
        CHECK_EQUAL(0x01, ((const uint8_t *)buf)[1]); // Address field
        CHECK_EQUAL(0x73, ((const uint8_t *)buf)[2]); // UA packet
        CHECK_EQUAL(0x7E, ((const uint8_t *)buf)[3]); // Flag
        return len; // Return number of bytes written
    });
    CHECK_EQUAL(TINY_SUCCESS, len);
}

TEST(TINY_FD, ABM_CheckMtuAPI)
{
    // Check MTU API
    int mtu = tiny_fd_get_mtu(handle);
    CHECK(mtu > 0); // MTU should be greater than 0
    CHECK_EQUAL(34, mtu); // Assuming the MTU is 34 bytes according to protocol test configuration
}
