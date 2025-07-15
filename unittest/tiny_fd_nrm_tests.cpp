/*
    Copyright 2019-2025 (C) Alexey Dynda

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

TEST_GROUP(TINY_FD_NRM)
{
    void setup()
    {
        connected = 0;
        tiny_fd_init_t init{};
        init.pdata = this;
        init.addr = TINY_FD_PRIMARY_ADDR; // Primary station address
        init.peers_count = 2;
        init.on_connect_event_cb = __onConnect;
        init.on_read_cb = onRead;
        init.on_send_cb = onSend;
        init.log_frame_cb = logFrame;
        init.buffer = inBuffer.data();
        init.buffer_size = inBuffer.size();
        init.window_frames = 7;
        init.send_timeout = 1000;
        init.retry_timeout = 100;
        init.retries = 2;
        init.mode = TINY_FD_MODE_NRM;
        init.crc_type = HDLC_CRC_OFF;
        auto result = tiny_fd_init(&handle, &init);
        CHECK_EQUAL(TINY_SUCCESS, result);
    }

    void teardown()
    {
        tiny_fd_close(handle);
        logFrameFunc = nullptr;
    }

    void onConnect(uint8_t, bool status) { connected = connected + (status ? 1: -1); }
    void onRead(uint8_t, uint8_t *, int) { }
    void onSend(uint8_t, const uint8_t *, int) { }

    static void __onConnect(void *udata, uint8_t address, bool connected)
    {
        // get the instance of the test class
        auto *self = static_cast<TEST_GROUP_CppUTestGroupTINY_FD_NRM *>(udata);
        self->onConnect(address, connected);
    }

    static void onRead(void *udata, uint8_t address, uint8_t *buf, int len)
    {
        // get the instance of the test class
        auto *self = static_cast<TEST_GROUP_CppUTestGroupTINY_FD_NRM *>(udata);
        self->onRead(address, buf, len);
    }

    static void onSend(void *udata, uint8_t address, const uint8_t *buf, int len)
    {
        // get the instance of the test class
        auto *self = static_cast<TEST_GROUP_CppUTestGroupTINY_FD_NRM *>(udata);
        self->onSend(address, buf, len);
    }

    static void logFrame(void *udata, tiny_fd_handle_t handle, tiny_fd_frame_direction_t direction,
                        tiny_fd_frame_type_t frame_type, tiny_fd_frame_subtype_t frame_subtype,
                        uint8_t ns, uint8_t nr, const uint8_t *data, int len)
    {
        // get the instance of the test class
        auto *self = static_cast<TEST_GROUP_CppUTestGroupTINY_FD_NRM *>(udata);
        if (!self->logFrameFunc) {
            return; // No logging function set
        }
        self->logFrameFunc(handle, direction, frame_type, frame_subtype, ns, nr, data, len);
    }

    tiny_fd_handle_t handle = nullptr;
    int connected = 0;
    std::array<uint8_t, 1024> inBuffer{};
    std::array<uint8_t, 1024> outBuffer{};
    std::function<void(tiny_fd_handle_t, tiny_fd_frame_direction_t,
                       tiny_fd_frame_type_t, tiny_fd_frame_subtype_t, uint8_t, uint8_t,
                       const uint8_t *, int)> logFrameFunc = nullptr;

    void establishConnection(uint8_t addr)
    {
        // We use emulatation of SNRM frame coming from secondary station
        // We force primary station to receive a token from secondary station: 0x3F
        // This will allow primary station to respond immediately with UA frame
        // FD protocol engine must be updated to handle U and I frame separately for each peer
        const uint8_t snrm_frame[] = {0x7E, (uint8_t)(0x01 | (addr << 2)), 0x3F, 0x7E}; // SNRM frame
        auto read_result = tiny_fd_on_rx_data(handle, snrm_frame, sizeof(snrm_frame));
        CHECK_EQUAL(TINY_SUCCESS, read_result);
        int len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
        CHECK_EQUAL(4, len);
        // Check UA frame
        CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
        CHECK_EQUAL(0x01 | (addr << 2), outBuffer[1]); // Address field
        CHECK_EQUAL(0x73, outBuffer[2]); // UA packet
        CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
    }
};

TEST(TINY_FD_NRM, NRM_ConnectionInitiatedFromPrimary)
{
    tiny_fd_register_peer(handle, 0x01);
    tiny_fd_register_peer(handle, 0x02);
    int len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(4, len);
    // Check SNRM frame
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x07, outBuffer[1]); // Address field: 0x01 peer, CR bit set
    CHECK_EQUAL(0x93, outBuffer[2]); // SNRM packet
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
    // Now emulate answer from secondary station
    auto read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x07\x73\x7E", 4); // UA frame
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    CHECK_EQUAL(1, connected); // Connection should be established

    len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(4, len);
    // Check SNRM frame
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x0B, outBuffer[1]); // Address field: 0x02 peer, CR bit set
    CHECK_EQUAL(0x93, outBuffer[2]); // SNRM packet
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
    // Now emulate answer from secondary station
    read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x0B\x73\x7E", 4); // UA frame
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    CHECK_EQUAL(2, connected); // Connection should be established
}

TEST(TINY_FD_NRM, NRM_ConnectInitiatedFromSecondary)
{
    tiny_fd_register_peer(handle, 0x01);
    tiny_fd_register_peer(handle, 0x02);
    // For command requests CR bit must be set, that is why address is 0x03
    auto read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x07\x2F\x7E", 4); // SNRM frame
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    int len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(4, len);
    // Check UA frame
    // UA frame should be: 0x7E 0x03 0x00 0x7E
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x05, outBuffer[1]); // Address field
    CHECK_EQUAL(0x73, outBuffer[2]); // UA packet
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
    CHECK_EQUAL(1, connected); // Connection should be established
}

TEST(TINY_FD_NRM, NRM_ConnectionWhenNoSecondaryStationIsRegistered)
{
    int len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(TINY_ERR_UNKNOWN_PEER, len);
}

TEST(TINY_FD_NRM, NRM_CheckUnitTestConnectionLogicForPrimary)
{
    // This test is to check that connection logic works correctly
    // when we are in NRM mode and no secondary station is registered.
    tiny_fd_register_peer(handle, 0x01);
    tiny_fd_register_peer(handle, 0x02);
    establishConnection(0x01);
    CHECK_EQUAL(1, connected); // Connection should be established
    establishConnection(0x02);
    CHECK_EQUAL(2, connected); // Connection should be established
}

TEST(TINY_FD_NRM, NRM_SecondaryDisconnection)
{
    tiny_fd_register_peer(handle, 0x01);
    establishConnection(0x01);
    CHECK_EQUAL(1, connected); // Connection should be established
    auto read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x07\x53\x7E", 4); // DISC frame
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    int len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(4, len);
    // Check UA frame   
    // UA frame should be: 0x7E 0x03 0x00 0x7E
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x05, outBuffer[1]); // address field - CR bit must be cleared
    CHECK_EQUAL(0x73, outBuffer[2]); // UA packet
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
    CHECK_EQUAL(0, connected); // Connection should be closed
}

#if 0
TEST(TINY_FD_NRM, NRM_DisconnectedState_PrimaryIgnoresAllFramesExcept_SNRM)
{
    tiny_fd_register_peer(handle, 0x01);
    // Send UA frame from secondary station to primary station
    // This should not change the connection state, because primary station is not connected
    auto read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x05\x73\x7E", 4); // UA frame
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    CHECK_EQUAL(0, connected); // Connection should not be established

    // Check that unknown source is ignored
    read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x25\x93\x7E", 4); // Unknown frame
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    CHECK_EQUAL(0, connected); // Connection should not be established

    // Now send SNRM frame from secondary station to primary station
    read_result = tiny_fd_on_rx_data(handle, (uint8_t *)"\x7E\x05\x93\x7E", 4); // SNRM frame
    CHECK_EQUAL(TINY_SUCCESS, read_result);
    int len = tiny_fd_get_tx_data(handle, outBuffer.data(), outBuffer.size(), 100);
    CHECK_EQUAL(4, len);
    // Check UA frame
    // UA frame should be: 0x7E 0x03 0x00 0x7E
    CHECK_EQUAL(0x7E, outBuffer[0]); // Flag
    CHECK_EQUAL(0x05, outBuffer[1]); // Address field
    CHECK_EQUAL(0x63, outBuffer[2]); // UA packet without poll
    CHECK_EQUAL(0x7E, outBuffer[3]); // Flag
    CHECK_EQUAL(1, connected); // Connection should be established   
}
#endif // 0
