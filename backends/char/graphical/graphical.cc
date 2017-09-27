/*
 *  This file is part of Rabbits
 *  Copyright (C) 2017  Clement Deschamps and Luc Michel
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

#include "graphical.h"

/* For Base64 encoding */
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

using std::string;

GraphicalCharBackend::GraphicalCharBackend(sc_core::sc_module_name n,
                                           const Parameters &p,
                                           ConfigManager &c)
    : Component(n, p, c), m_port("char")
{
    std::string index_path;

    try {
        index_path = c.get_resource_manager()
            .get_inventory("backend-chardev-graphical")
            .get_resource("index.html")
            .get_absolute_uri();
    } catch (ResourceNotFoundException &e) {
        MLOG(APP, WRN) << "Graphical chardev backend resource files not found\n";
        return;
    }

    m_webkit = c.get_ui().create_webkit("char", index_path.c_str());

    if (!m_webkit) {
        MLOG(APP, DBG) << "Graphical chardev backend creation failed (no graphic mode?)\n";
        return;
    }

    m_webkit->register_event_listener(*this);
    m_port.register_binding_listener(*this);

    SC_THREAD(recv_thread);
    SC_THREAD(send_thread);
}

GraphicalCharBackend::~GraphicalCharBackend()
{
}

void GraphicalCharBackend::port_binding_event(Port &this_port,
                                              Port &peer_port)
{
    string n("Console: ");
    n += peer_port.get_parent()->get_component().get_name();

    m_webkit->set_name(n);
}

void GraphicalCharBackend::recv_thread()
{
    using namespace boost::archive::iterators;

    std::vector<uint8_t> data;

    for(;;) {
        m_port.recv(data);

        /* base64 iterator */
        typedef base64_from_binary<transform_width<const char *, 6, 8> > text_to_base64;

        std::stringstream os;
        std::copy(text_to_base64(&data[0]),
                  text_to_base64(&data[0] + data.size()),
                  ostream_iterator<char>(os));

        m_webkit->exec_js(std::string("writeToTerminal(\"")
                          + os.str()
                          + "\");");
    }
}

void GraphicalCharBackend::webkit_event(const std::string & event)
{
    using namespace boost::archive::iterators;

    typedef transform_width<binary_from_base64<remove_whitespace
        <std::string::const_iterator> >, 8, 6> base64_to_text;

    try {
        std::string input = event;
        // If the input isn't a multiple of 4, pad with =
        size_t num_pad_chars((4 - input.size() % 4) % 4);
        input.append(num_pad_chars, '=');

        size_t pad_chars(std::count(input.begin(), input.end(), '='));
        std::replace(input.begin(), input.end(), '=', 'A');

        std::string output(base64_to_text(input.begin()),
                           base64_to_text(input.end()));

        output.erase(output.end() - pad_chars, output.end());

        m_ev_mutex.lock();
        std::copy(output.begin(), output.end(), std::back_inserter(m_buf));
        m_ev_mutex.unlock();

    } catch (std::exception const&) {
        MLOG(APP, DBG) << "Error while decoding base64 input from javascript\n";
    }
}

void GraphicalCharBackend::send_thread()
{
    for(;;) {
        sc_core::wait(10, sc_core::SC_US);

        m_ev_mutex.lock();

        if (!m_buf.empty()) {
            m_port.send(m_buf);
            m_buf.clear();
        }

        m_ev_mutex.unlock();
    }
}

