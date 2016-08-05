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
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "serial.h"

void SerialCharBackend::recv_thread()
{
    std::vector<uint8_t> data;

    if(m_fd < 0) {
        return;
    }

    for(;;) {
        m_port.recv(data);

        MLOG(SIM, TRC) << "Got " << int(data[0]) << "(" << data[0] << ")\n";

        ::write(m_fd, &data[0], data.size());
    }
}

void SerialCharBackend::send_thread()
{
    struct pollfd fd;
    
    if(m_fd < 0) {
        return;
    }

    fd.fd = m_fd;
    fd.events = POLLIN | POLLPRI;

    m_buf.resize(256);

    for(;;) {
        sc_core::wait(10, sc_core::SC_US);

        int ret = poll(&fd, 1, 0);

        if (ret > 0) {
            int ret = ::read(m_fd, &m_buf[0], m_buf.size());

            if (ret < 0 && errno != EINTR) {
                MLOG(APP, ERR) << "read failed: " << std::strerror(errno) << "\n";
                abort();
            }
            else if(ret > 0) {
                m_buf.resize(ret);
                m_port.send(m_buf);
                m_buf.resize(256);
            }
        } else if (ret == EINVAL) {
            MLOG(APP, ERR) << "poll failed: " << std::strerror(errno) << "\n";
            abort();
        }
    }
}

void SerialCharBackend::open(std::string dev)
{
    m_fd = ::open(dev.c_str(), O_RDWR);

    if(m_fd < 0) {
        MLOG(APP, ERR) << "open failed (" << dev << "): " << std::strerror(errno) << "\n";
        return;
    }

    MLOG(APP, INF) << "opened: " << dev << " --> " << m_fd << "\n";
}

void SerialCharBackend::close()
{
    if(m_fd >= 0) {
        ::close(m_fd);
    }
    m_fd = -1;
}
