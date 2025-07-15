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

#pragma once

#include "tiny_fd.h"
#include "tiny_fd_frames_int.h"
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////

tiny_fd_frame_info_t *__put_u_s_frame_to_tx_queue(tiny_fd_handle_t handle, int type, const void *data, int len);

///////////////////////////////////////////////////////////////////////////////

uint8_t* tiny_fd_get_next_s_u_frame_to_send(tiny_fd_handle_t handle, int *len, uint8_t peer, uint8_t address);

///////////////////////////////////////////////////////////////////////////////
