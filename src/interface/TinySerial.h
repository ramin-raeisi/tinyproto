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

#pragma once

#include "hal/tiny_serial.h"

#if defined(ARDUINO)
#include <Arduino.h>
#endif
#include <stdint.h>
#include <limits.h>

namespace tinyproto
{

class Serial
{
public:
#if defined(ARDUINO)
    explicit Serial(HardwareSerial &dev);
#endif
    explicit Serial(const char *dev);

    void setTimeout(uint32_t timeoutMs);

    bool begin(uint32_t speed);

    void end();

    int readBytes(uint8_t *buf, int len);

    int write(const uint8_t *buf, int len);

private:
    const char *m_dev;
    tiny_serial_handle_t m_handle = TINY_SERIAL_INVALID;
    uint32_t m_timeoutMs = 0;
};

} // namespace tinyproto
