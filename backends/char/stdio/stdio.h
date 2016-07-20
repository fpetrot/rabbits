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

#ifndef _BACKEND_CHAR_STDIO_H
#define _BACKEND_CHAR_STDIO_H

#include <vector>

#include <rabbits/component/component.h>
#include <rabbits/component/port/char.h>

#include <termios.h>

class StdioCharBackend : public Component {
public:
    /* Escape character is ctrl-a */
    const uint8_t ESCAPE = 0x01;
private:
    static bool in_use;

    CharPort m_port;

    void send_char(uint8_t c);
    void handle_escape(uint8_t c);

    void send_buf();

    void recv_thread();
    void send_thread();

    void setup_tty();
    void restore_tty();

    std::vector<uint8_t> m_buf;

    termios m_tty_all_save;
    termios m_tty_out_save;

    bool m_got_escape = false;

public:
    SC_HAS_PROCESS(StdioCharBackend);
    StdioCharBackend(sc_core::sc_module_name n, Parameters &p, ConfigManager &c)
        : Component(n, p, c), m_port("char")
    {
        if (in_use) {
            LOG(APP, ERR) << "Only one stdio char backend allowed\n";
            return;
        }

        setup_tty();

        in_use = true;

        SC_THREAD(recv_thread);
        SC_THREAD(send_thread);
    }

    virtual ~StdioCharBackend()
    {
        restore_tty();
        in_use = false;
    }
};

#endif
