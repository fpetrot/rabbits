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
#ifndef _RABBITS_COMPONENT_CHANNEL_I2C_H
#define _RABBITS_COMPONENT_CHANNEL_I2C_H

#include <systemc>
#include <inttypes.h>
#include <vector>

struct I2CFrame {
    enum eDirection {
        WRITE, READ
    };

    bool valid = false;
    bool stop = false;
    uint16_t addr;
    eDirection direction;
    std::vector<uint8_t> send_data;
    std::vector<uint8_t> recv_data;

    void clear() {
        send_data.clear();
        recv_data.clear();
        valid = false;
        stop = false;
    }
};

class I2CSystemCInterface : public virtual sc_core::sc_interface {
public:
    virtual void i2c_slave_xmit(I2CFrame &) = 0;
};

#endif
