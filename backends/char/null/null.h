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

#ifndef _CONSOLE_BACKEND_H
#define _CONSOLE_BACKEND_H

#include <vector>

#include <rabbits/component/component.h>
#include <rabbits/component/port/uart.h>


class NullCharDevice : public Component{
private:
    UartPort m_port;

    void recv_thread()
    {
        std::vector<uint8_t> data;

        for(;;) {
            m_port.recv(data);
        }
    }

public:
    SC_HAS_PROCESS(NullCharDevice);
    NullCharDevice(sc_core::sc_module_name n, ComponentParameters &p) 
        : Component(n, p), m_port("uart")
    {
        SC_THREAD(recv_thread);
    }
};

#endif
