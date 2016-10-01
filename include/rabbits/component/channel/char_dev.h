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

#ifndef _RABBITS_COMPONENT_CHANNEL_CHAR_DEV_H
#define _RABBITS_COMPONENT_CHANNEL_CHAR_DEV_H

#include <systemc>

class CharDeviceSystemCInterface : public virtual sc_core::sc_interface {
public:
    virtual void send(std::vector<uint8_t> &data) = 0;
    virtual void recv(std::vector<uint8_t> &data) = 0;
    virtual bool empty() const = 0;
};


class CharDeviceChannel : public CharDeviceSystemCInterface,
                          public sc_core::sc_prim_channel
{
private:
    std::vector<uint8_t> m_buffer;

    sc_core::sc_event m_recv_ev;

public:
    void send(std::vector<uint8_t> &data)
    {
        m_buffer.insert(m_buffer.end(), data.begin(), data.end());
        request_update();
    }

    void update()
    {
        m_recv_ev.notify(sc_core::SC_ZERO_TIME);
    }

    void recv(std::vector<uint8_t> &data)
    {
        if (m_buffer.empty()) {
            sc_core::wait(m_recv_ev);
        }

        /* TODO: avoid data copy */
        data.clear();
        data.insert(data.end(), m_buffer.begin(), m_buffer.end());
        m_buffer.clear();
    }

    bool empty() const {
        return m_buffer.empty();
    }
};
#endif
