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

#include "tiny_fd_service_queue_int.h"

#include "tiny_fd.h"
#include "tiny_fd_int.h"
#include "tiny_fd_defines_int.h"
#include "tiny_fd_peers_int.h"

///////////////////////////////////////////////////////////////////////////////

tiny_fd_frame_info_t *__put_u_s_frame_to_tx_queue(tiny_fd_handle_t handle, int type, const void *data, int len)
{
    tiny_fd_frame_info_t *slot = tiny_fd_queue_allocate( &handle->frames.s_queue, type, ((const uint8_t *)data) + 2, len - 2 );
    // Check if space is actually available
    if ( slot != NULL )
    {
        slot->header.address = ((const uint8_t *)data)[0];
        slot->header.control = ((const uint8_t *)data)[1];
        LOG(TINY_LOG_DEB, "[%p] QUEUE SU-PUT: [%02X] [%02X]\n", handle, slot->header.address, slot->header.control);
        tiny_events_set(&handle->events, FD_EVENT_TX_DATA_AVAILABLE);
        return slot;
    }
    else
    {
        // Not enough space in the queue, so we cannot put the frame to the queue
        LOG(TINY_LOG_WRN, "[%p] Not enough space for S- U- Frames. Retransmissions may occur\n", handle);
    }
    return slot;
}

///////////////////////////////////////////////////////////////////////////////

uint8_t *tiny_fd_get_next_s_u_frame_to_send(tiny_fd_handle_t handle, int *len, uint8_t peer, uint8_t address)
{
    uint8_t *data = NULL;
    // LOG(TINY_LOG_DEB, "[%p] QUEUE SEARCH: [%02X] [%02X]\n", handle, address, TINY_FD_QUEUE_S_FRAME | TINY_FD_QUEUE_U_FRAME);
    tiny_fd_frame_info_t *ptr = tiny_fd_queue_get_next( &handle->frames.s_queue, TINY_FD_QUEUE_S_FRAME | TINY_FD_QUEUE_U_FRAME, address, 0 );
    if ( ptr != NULL )
    {
        // clear queue only, when send is done, so for now, use pointer data for sending only
        data = (uint8_t *)&ptr->header;
        *len = ptr->len + sizeof(tiny_frame_header_t);
        if ( (data[1] & HDLC_S_FRAME_MASK) == HDLC_S_FRAME_BITS )
        {
            handle->peers[peer].sent_nr = ptr->header.control >> 5;
        }
#if TINY_FD_DEBUG
        if ( (data[1] & HDLC_U_FRAME_MASK) == HDLC_U_FRAME_BITS )
        {
            LOG(TINY_LOG_INFO, "[%p] Sending U-Frame type=%02X with address [%02X] to %s\n", handle, data[1] & HDLC_U_FRAME_TYPE_MASK, data[0],
                      __is_primary_station( handle ) ? "secondary" : "primary");
        }
        else if ( (data[1] & HDLC_S_FRAME_MASK) == HDLC_S_FRAME_BITS )
        {
            LOG(TINY_LOG_INFO, "[%p] Sending S-Frame N(R)=%02X, type=%s with address [%02X] to %s\n", handle, data[1] >> 5,
                ((data[1] >> 2) & 0x03) == 0x00 ? "RR" : "REJ", data[0],  __is_primary_station( handle ) ? "secondary" : "primary");
        }
#endif
    }
    return data;
}

///////////////////////////////////////////////////////////////////////////////
