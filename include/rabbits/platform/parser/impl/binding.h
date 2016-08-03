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

#ifndef _RABBITS_PLATFORM_PARSER_IMPL_BINDING_H
#define _RABBITS_PLATFORM_PARSER_IMPL_BINDING_H

#include <cassert>

#include "../binding.h"
#include "../module.h"
#include "../platform.h"

#include "rabbits/component/port.h"

#include "rabbits/logger.h"

inline void ParserNodeBinding::parse_peer(std::string s_peer)
{
    const Namespace * ns = & m_parent.get_namespace();

    /* Namespace parsing */
    std::size_t pos = s_peer.find(':');
    if (pos != std::string::npos) {
        const std::string ns_s = s_peer.substr(0, pos);
        try {
            ns = &(Namespace::find_by_name(ns_s));
        } catch (NamespaceNotFoundException e) {
            throw NamespaceNotFoundParseException(get_descr(), ns_s);

        }

        s_peer = s_peer.substr(pos+1, std::string::npos);
    }

    /* Module name parsing */
    pos = s_peer.find('.');
    const std::string mod_s = s_peer.substr(0, pos);

    if (!get_root().module_with_ports_exists(*ns, mod_s)) {
        throw ModuleNotFoundParseException(get_descr(), *ns, mod_s);
    }

    m_peer = get_root().get_module_with_ports(*ns, mod_s);

    /* Port name parsing */
    if (pos != std::string::npos) {
        /* Port is specified */
        m_peer_port_s = s_peer.substr(pos+1, std::string::npos);
    }
}

inline ParserNodeBinding::ParserNodeBinding(PlatformDescription &descr,
                                            const std::string &n,
                                            ParserNodePlatform &root,
                                            ParserNodeModuleWithPorts &parent)
    : ParserNode(descr, root), m_parent(parent), m_local_port_s(n)
{}

inline ParserNodeBinding::ParserNodeBinding(const std::string &local_port,
                                            std::shared_ptr<ParserNodeModuleWithPorts> peer,
                                            const std::string &peer_port,
                                            PlatformDescription &params,
                                            ParserNodePlatform &root,
                                            ParserNodeModuleWithPorts &parent)
    : ParserNode(root), m_parent(parent), m_local_port_s(local_port)
    , m_peer(peer), m_peer_port_s(peer_port)
{}

inline ParserNodeBinding::~ParserNodeBinding() {}

inline void ParserNodeBinding::second_pass()
{
    PlatformDescription descr = get_descr();

    if (descr.is_scalar()) {
        parse_peer(descr.as<std::string>());
    } else if (descr.is_map()) {
        if (!descr.exists("peer")) {
            throw MissingFieldParseException(descr, "peer");
        }

        parse_peer(descr["peer"].as<std::string>());

    } else {
        throw PlatformParseException(descr, "Invalid peer specification");
    }

    ParserNode::second_pass();
}

inline void ParserNodeBinding::instanciation_done()
{
    assert(m_parent.get_inst() != nullptr);
    assert(m_peer->get_inst() != nullptr);

    HasPortIface &local = *(m_parent.get_inst());
    HasPortIface &peer = *(m_peer->get_inst());

    if (!local.port_exists(m_local_port_s)) {
        throw PortNotFoundParseException(get_descr(), m_parent.get_namespace(),
                                         m_parent.get_name(), m_local_port_s);
    }

    m_local_port = &local.get_port(m_local_port_s);


    if (m_peer_port_s.empty()) {
        /* When no port is specified, we take the first available port */
        auto p = peer.port_begin();
        if (p == peer.port_end()) {
            throw NoPortFoundParseException(get_descr(), m_peer->get_namespace(),
                                            m_peer->get_name());
        }

        m_peer_port_s = p->first;
        m_peer_port = p->second;
    } else if (!peer.port_exists(m_peer_port_s)) {
        throw PortNotFoundParseException(get_descr(), m_peer->get_namespace(),
                                         m_peer->get_name(), m_peer_port_s);
    } else {
        m_peer_port = &peer.get_port(m_peer_port_s);
    }

    ParserNode::instanciation_done();
}

inline ParserNodeModuleWithPorts & ParserNodeBinding::get_peer() const
{
    assert(m_peer != nullptr);
    return *m_peer;
}


#endif
