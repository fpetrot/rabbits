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

#ifndef _BACKEND_CHAR_SOCKET_H
#define _BACKEND_CHAR_SOCKET_H

#include <vector>

#include <rabbits/component/component.h>
#include <rabbits/component/port/char.h>

class SocketCharBackend : public Component {
private:
    CharPort m_port;

    void send_char(uint8_t c);

    void recv_thread();
    void send_thread();

    void setup_tcp_server(std::string ip, std::string port);
    void setup_tcp_client(std::string ip, std::string port);
    void close();

    bool m_server;
    int m_srv_socket;
    bool m_nowait;
    int m_socket = -1;
    std::vector<uint8_t> m_buf;

public:
    SC_HAS_PROCESS(SocketCharBackend);
    SocketCharBackend(sc_core::sc_module_name n, const Parameters &p, ConfigManager &c)
        : Component(n, p, c), m_port("char")
    {
        std::string type = p["kind"].as<std::string>();
        std::string address = p["address"].as<std::string>();
        m_server = p["server"].as<bool>();
        m_nowait = p["nowait"].as<bool>();

        if(type == "tcp") {
            std::string ip = "127.0.0.1";
            std::string port = "4001";

            size_t count = std::count(address.begin(), address.end(), ':');
            if(count == 1) {
                size_t first = address.find_first_of(':');
                ip = address.substr(0, first);
                port = address.substr(first + 1);
            }
            else {
                // error
                MLOG(APP, ERR) << "malformed address, expecting IP:PORT (e.g 127.0.0.1:4001)\n";
                return;
            }

            MLOG(APP, DBG) << "IP: " << ip << ", PORT: " << port << "\n";

            if(m_server) {
                setup_tcp_server(ip, port);
            }
            else {
                setup_tcp_client(ip, port);
            }
        }
        else if(type == "udp") {
            MLOG(APP, ERR) << "udp sockets are not available in this version\n";
            return;
        }
        else if(type == "unix") {
            MLOG(APP, ERR) << "unix sockets are not available in this version\n";
            return;
        }
        else {
            MLOG(APP, ERR) << "bad value for socket type\n";
            return;
        }

        SC_THREAD(recv_thread);
        SC_THREAD(send_thread);
    }

    virtual ~SocketCharBackend()
    {
        close();
    }
};

#endif
