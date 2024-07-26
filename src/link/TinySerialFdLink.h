/*
    Copyright 2016-2024 (C) Alexey Dynda

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

#pragma once

#include "TinySerialLinkLayer.h"
#include "TinyFdLinkLayer.h"

#if defined(ARDUINO)
#include "proto/fd/tiny_fd_int.h"
#endif

namespace tinyproto
{

template <int MTU, int TX_WINDOW, int BUFFER_SIZE, int BLOCK> class StaticSerialFdLink: public ISerialLinkLayer<IFdLinkLayer, BLOCK>
{
public:
    explicit StaticSerialFdLink(char *dev)
        : ISerialLinkLayer<IFdLinkLayer, BLOCK>(dev, this->m_buffer, BUFFER_SIZE)
    {
        this->setMtu(MTU);
        this->setWindow(TX_WINDOW);
    }

private:
    uint8_t m_buffer[BUFFER_SIZE] = {};
};


#if defined(ARDUINO)

/** Valid only for Arduino IDE, since it has access to internal headers */
template <int MTU, int TX_WINDOW, int RX_WINDOW, int BLOCK> using ArduinoStaticSerialFdLinkLayer = StaticSerialFdLink<MTU, TX_WINDOW, FD_BUF_SIZE_EX(MTU, TX_WINDOW, HDLC_CRC_16, RX_WINDOW), BLOCK>;

class ArduinoSerialFdLink: public ArduinoStaticSerialFdLinkLayer<32, 2, 2, 4>
{
public:
    explicit ArduinoSerialFdLink(HardwareSerial *dev)
        : ArduinoStaticSerialFdLinkLayer<32, 2, 2, 4>(reinterpret_cast<char *>(dev))
    {
    }
};

#endif

class SerialFdLink: public ISerialLinkLayer<IFdLinkLayer, 32>
{
public:
    explicit SerialFdLink(char *dev)
        : ISerialLinkLayer<IFdLinkLayer, 32>(dev, nullptr, 0)
    {
    }

    ~SerialFdLink();

    bool begin(on_frame_read_cb_t onReadCb, on_frame_send_cb_t onSendCb, void *udata) override;

    void end() override;

private:
    uint8_t *m_buffer = nullptr;
};

} // namespace tinyproto
