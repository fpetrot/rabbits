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

#ifndef _BACKEND_CHAR_GRAPHICAL_H
#define _BACKEND_CHAR_GRAPHICAL_H

#include <mutex>
#include <vector>

#include <rabbits/component/component.h>
#include <rabbits/component/port/char.h>

#include <rabbits/ui/ui.h>

class GraphicalCharBackend : public Component
                           , public UiWebkitEventListener
                           , public PortBindingListener {
private:
    UiViewWebkitIface *m_webkit = nullptr;

    std::mutex m_ev_mutex;
    std::vector<uint8_t> m_buf;

    void recv_thread();
    void send_thread();

public:
    SC_HAS_PROCESS(GraphicalCharBackend);
    GraphicalCharBackend(sc_core::sc_module_name n, const Parameters &p, ConfigManager &c);

    virtual ~GraphicalCharBackend();

    /* UiWebkitEventListener */
    void webkit_event(const std::string & event);

    /* PortBindingListener */
    void port_binding_event(Port &this_port, Port &peer_port);

    CharPort m_port;
};

#endif
