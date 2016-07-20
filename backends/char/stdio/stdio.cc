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

#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <cstring>

#include "stdio.h"

bool StdioCharBackend::in_use = false;

void StdioCharBackend::recv_thread()
{
    std::vector<uint8_t> data;

    for(;;) {
        m_port.recv(data);

        MLOG(SIM, TRC) << "Got " << int(data[0]) << "(" << data[0] << ")\n";

        write(1, &data[0], data.size());
    }
}

void StdioCharBackend::send_char(uint8_t c)
{
    std::vector<uint8_t> data;
    data.push_back(c);

    m_port.send(data);
}

void StdioCharBackend::handle_escape(uint8_t c)
{
    MLOG(SIM, TRC) << "Got escape: " << c << "\n";

    if (c == 'x') {
        sc_core::sc_stop();
    } else if (c == ESCAPE) {
        /* When hitting ctrl-a ctrl-a, just forward 'ctrl-a' */
        send_char(ESCAPE);
    }
}

void StdioCharBackend::send_buf()
{
    for (uint8_t c : m_buf) {
        if ((!m_got_escape) && (c == ESCAPE)) {
            m_got_escape = true;
            continue;
        }

        if (m_got_escape) {
            handle_escape(c);
            m_got_escape = false;
        } else {
            send_char(c);
        }
    }
}

void StdioCharBackend::send_thread()
{
    struct pollfd fd;

    fd.fd = 0;
    fd.events = POLLIN | POLLPRI;

    m_buf.resize(256);

    for(;;) {
        sc_core::wait(10, sc_core::SC_US);

        int ret = poll(&fd, 1, 0);

        if (ret > 0) {
            int ret = read(0, &m_buf[0], m_buf.size());

            if (ret < 0 && errno != EINTR) {
                MLOG(APP, ERR) << "read failed: " << std::strerror(errno) << "\n";
                abort();
            }

            m_buf.resize(ret);
            send_buf();
            m_buf.resize(256);

        } else if (ret == EINVAL) {
            MLOG(APP, ERR) << "poll failed: " << std::strerror(errno) << "\n";
            abort();
        }
    }
}

void StdioCharBackend::setup_tty()
{
    termios tty;

    tcgetattr(0, &m_tty_all_save);

    tty = m_tty_all_save;

    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                     |INLCR|IGNCR|ICRNL|IXON);

    tty.c_oflag |= OPOST;

    tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN|ISIG);

    tty.c_cflag &= ~(CSIZE|PARENB);
    tty.c_cflag |= CS8;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    tcsetattr(0, TCSANOW, &tty);
}

void StdioCharBackend::restore_tty()
{
    MLOG(APP, DBG) << "Restoring tty state\n";
    tcsetattr(0, TCSANOW, &m_tty_all_save);
}
