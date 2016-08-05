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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <netdb.h>

#include "socket.h"

using std::string;

void SocketCharBackend::recv_thread()
{
    std::vector<uint8_t> data;

	if(!m_nowait && ((!m_server && m_socket < 0) || (m_server && m_srv_socket < 0))) {
		return;
	}

    for(;;) {
		if(m_nowait && m_socket < 0) {
        	sc_core::wait(1, sc_core::SC_MS);
			continue;
		}

        m_port.recv(data);

        MLOG(SIM, INF) << "Got " << int(data[0]) << "(" << data[0] << ")\n";

        ::write(m_socket, &data[0], data.size());
    }
}

void SocketCharBackend::send_thread()
{
	if(!m_nowait && ((!m_server && m_socket < 0) || (m_server && m_srv_socket < 0))) {
		return;
	}

    m_buf.resize(256);

    for(;;) {
        sc_core::wait(10, sc_core::SC_US);

		if(m_nowait && m_server && m_socket < 0) {
    		socklen_t addr_len = sizeof(struct sockaddr_in);
			struct sockaddr_in client_addr;
			int flag;

			m_socket = ::accept(m_srv_socket, (struct sockaddr*)&client_addr, &addr_len);
			if(m_socket < 0) {
				continue;
			}

			flag = 1;
			if(::ioctl(m_socket, FIONBIO, (char *)&flag) < 0) {
				MLOG(APP, ERR) << "setting socket in blocking mode failed: " << std::strerror(errno) << "\n";
				continue;
			}

			flag = 1;
			if (::setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag))) {
				MLOG(APP, WRN) << "setting up TCP_NODELAY option failed: " << std::strerror(errno) << "\n";
			}

			char str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(client_addr.sin_addr), str, INET_ADDRSTRLEN);
			int cport = ntohs(client_addr.sin_port);

			MLOG(APP, INF) << "incoming connection from  " << str << ":" << cport << "\n";
		}
		else if(m_nowait && !m_server && m_socket < 0) {
			// TODO
			continue;
		}

		
		int ret = ::read(m_socket, &m_buf[0], m_buf.size());
		if (ret < 0 && errno != EAGAIN && errno != EINTR) {
			MLOG(APP, ERR) << "read failed: " << std::strerror(errno) << "\n";
			abort();
		}
		else if(ret > 0) {
			m_buf.resize(ret);
			m_port.send(m_buf);
			m_buf.resize(256);
		}
    }
}

void SocketCharBackend::setup_tcp_server(string ip, string port)
{
	int ret;
    struct sockaddr_in addr_in;
    int iport;
	int flag;

	MLOG(APP, INF) << "setting up TCP server on " << ip << ":" << port << "\n";

    socklen_t addr_len = sizeof(struct sockaddr_in);

    m_srv_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if(m_srv_socket < 0) {
        MLOG(APP, ERR) << "socket failed: " << std::strerror(errno) << "\n";
        return;
    }

    flag = 1;
    ::setsockopt(m_srv_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    iport = std::stoi(port);

	::memset((char *)&addr_in, 0, sizeof(addr_in));
	addr_in.sin_family = AF_INET;
	addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_in.sin_port = htons(iport);

	ret = ::bind(m_srv_socket, (struct sockaddr *)&addr_in, sizeof(addr_in));
	if (ret < 0) {
		::close(m_srv_socket);
		m_srv_socket = -1;
        MLOG(APP, ERR) << "bind failed: " << std::strerror(errno) << "\n";
		return;
	} 

    ret = listen(m_srv_socket, 1);
    if (ret < 0) {
        ::close(m_srv_socket);
        m_srv_socket = -1;
        MLOG(APP, ERR) << "listen failed: " << std::strerror(errno) << "\n";
        return;
    }

    if(m_nowait) {
        flag = 1;
        if(::ioctl(m_srv_socket, FIONBIO, (char *)&flag) < 0) {
        	MLOG(APP, ERR) << "setting socket in non-blocking mode failed: " << std::strerror(errno) << "\n";
            return;
		}

		// accept() will be done later in the SC_THREAD
    }
    else {
        MLOG(APP, INF) << "waiting for a connection on " << ip << ":" << port << "\n";

        struct sockaddr_in client_addr;

        m_socket = ::accept(m_srv_socket, (struct sockaddr*)&client_addr, &addr_len);
        if(m_socket < 0) {
            ::close(m_srv_socket);
            m_srv_socket = -1;
            MLOG(APP, ERR) << "accept failed: " << std::strerror(errno) << "\n";
        }

        flag = 1;
        if(::ioctl(m_socket, FIONBIO, (char *)&flag) < 0) {
        	MLOG(APP, ERR) << "setting socket in blocking mode failed: " << std::strerror(errno) << "\n";
            return;
        }

		flag = 1;
		if (::setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag))) {
			MLOG(APP, WRN) << "setting up TCP_NODELAY option failed: " << std::strerror(errno) << "\n";
		}
        
        char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(client_addr.sin_addr), str, INET_ADDRSTRLEN);
        int cport = ntohs(client_addr.sin_port);

        MLOG(APP, INF) << "incoming connection from  " << str << ":" << cport << "\n";
    }
}

void SocketCharBackend::setup_tcp_client(string ip, string port)
{
	int flag = 1;
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	
	MLOG(APP, INF) << "setting up TCP client connection to " << ip << ":" << port << "\n";

	m_socket = ::socket(AF_INET, SOCK_STREAM, 0);

	if(m_socket == -1) {
        MLOG(APP, ERR) << "socket failed: " << std::strerror(errno) << "\n";
        return;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	status = getaddrinfo(ip.c_str(), port.c_str(), &hints, &servinfo);
	if(status == -1) {
        MLOG(APP, ERR) << "getaddrinfo failed: " << std::strerror(errno) << "\n";
        goto close_sock;
	}

	if(m_nowait) {
        flag = 1;
        if(::ioctl(m_socket, FIONBIO, (char *)&flag) < 0) {
        	MLOG(APP, ERR) << "setting socket in non-blocking mode failed: " << std::strerror(errno) << "\n";
            goto close_sock;
		}

		// connect() will be done later in the SC_THREAD
	}
	else  {
		if (::connect(m_socket, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
			MLOG(APP, ERR) << "connect failed: " << std::strerror(errno) << "\n";
			freeaddrinfo(servinfo);
			goto close_sock;
		}
		freeaddrinfo(servinfo);

		flag = 1;
        if(::ioctl(m_socket, FIONBIO, (char *)&flag) < 0) {
        	MLOG(APP, ERR) << "setting socket in blocking mode failed: " << std::strerror(errno) << "\n";
            return;
        }

		if (::setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag))) {
			MLOG(APP, WRN) << "setting up TCP_NODELAY option failed: " << std::strerror(errno) << "\n";
		}
	}

	return;

close_sock:
	::close(m_socket);
	m_socket = -1;
}

void SocketCharBackend::close()
{
    if(m_socket >= 0) {
        ::close(m_socket);
    }
    m_socket = -1;
}
