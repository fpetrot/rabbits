/*
 *  This file is part of Rabbits
 *  Copyright (C) 2015  Clement Deschamps and Luc Michel
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef _RABBITS_COMPONENT_CHANNEL_SPI_H
#define _RABBITS_COMPONENT_CHANNEL_SPI_H

#include <systemc>
#include <inttypes.h>
#include <vector>

#include "rabbits/logger.h"

struct SpiFrame {
    std::vector<uint8_t> send_data;
    std::vector<uint8_t> recv_data;

    unsigned int send_idx = 0;

    uint8_t send_pop()
    {
        if (send_data.size() <= send_idx) {
            LOG(APP, DBG) << "SPI frame: sent data underflow!\n";
            return 0;
        }

        return send_data[send_idx++];
    }

    unsigned int send_size()
    {
        return send_data.size() - send_idx;
    }

    bool send_empty() const
    {
        return send_data.size() == send_idx;
    }

    void send_clear()
    {
        send_idx = send_data.size();
    }

    void recv_push(uint8_t data)
    {
        if (recv_data.size() >= send_data.size()) {
            LOG(APP, DBG) << "SPI frame: slave tried to respond with more data than sent! Dropping\n";
            return;
        }

        recv_data.push_back(data);
    }

    void clear() {
        send_data.clear();
        recv_data.clear();
        send_idx = 0;
    }
};

class SpiSystemCInterface : public virtual sc_core::sc_interface {
public:
    virtual void spi_slave_xmit(SpiFrame &) = 0;
    virtual void spi_select() = 0;
    virtual void spi_deselect() = 0;
};

#endif
