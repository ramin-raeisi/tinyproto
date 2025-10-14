/*
    Copyright 2021-2024 (C) Alexey Dynda

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

#include "CustomSerialFdLink.h"
#include <stdlib.h>

namespace tinyproto
{

CustomSerialFdLink::~CustomSerialFdLink()
{
    if ( m_buffer )
    {
        free(m_buffer);
        m_buffer = nullptr;
    }
}

bool CustomSerialFdLink::begin(on_frame_read_cb_t onReadCb, on_frame_send_cb_t onSendCb, void *udata)
{
    int size = tiny_fd_buffer_size_by_mtu_ex(1, getMtu(), getWindow(), getCrc(), 3);
    m_buffer = reinterpret_cast<uint8_t *>(malloc(size));
    setBuffer(m_buffer, size);
    return ISerialLinkLayer<IFdLinkLayer, 32>::begin(onReadCb, onSendCb, udata);
}

void CustomSerialFdLink::end()
{
    ISerialLinkLayer<IFdLinkLayer, 32>::end();
    if ( m_buffer )
    {
        free(m_buffer);
        m_buffer = nullptr;
    }
}

void CustomSerialFdLink::runRx()
{
    uint8_t buf[maximumBlockSize];
    uint8_t *p = buf;

    int len = _rx.receive(p, maximumBlockSize);
    while ( len > 0 )
    {
        int temp = IFdLinkLayer::parseData(p, len);
        if ( temp < 0 )
        {
            break;
        }
        len -= temp;
        p += temp;
    }
}

void CustomSerialFdLink::runTx()
{
    uint8_t buf[maximumBlockSize];
    int len = IFdLinkLayer::getData(buf, maximumBlockSize);
    uint8_t *ptr = buf;
    while ( len > 0 )
    {
        int sent = _tx.send(ptr, len);
        if ( sent < 0 )
        {
            break;
        }
        ptr += sent;
        len -= sent;
    }
}

} // namespace tinyproto
