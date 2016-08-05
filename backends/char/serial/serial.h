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

#ifndef _BACKEND_CHAR_SERIAL_H
#define _BACKEND_CHAR_SERIAL_H

#include <vector>

#include <rabbits/component/component.h>
#include <rabbits/component/port/char.h>

class SerialCharBackend : public Component {
private:
    CharPort m_port;

    void send_char(uint8_t c);

    void recv_thread();
    void send_thread();

    void open(std::string dev);
    void close();

    int m_fd = -1;
    std::vector<uint8_t> m_buf;

public:
    SC_HAS_PROCESS(SerialCharBackend);
    SerialCharBackend(sc_core::sc_module_name n, const Parameters &p, ConfigManager &c)
        : Component(n, p, c), m_port("char")
    {
        std::string dev = p["path"].as<std::string>();

        open(dev);

        SC_THREAD(recv_thread);
        SC_THREAD(send_thread);
    }

    virtual ~SerialCharBackend()
    {
        close();
    }
};

#endif
