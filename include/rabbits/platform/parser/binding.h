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

#ifndef _RABBITS_PLATFORM_PARSER_BINDING_H
#define _RABBITS_PLATFORM_PARSER_BINDING_H

#include <cassert>

#include "node.h"

class ParserNodeModuleWithPorts;
class Port;

class ParserNodeBinding : public ParserNode {
    ParserNodeModuleWithPorts & m_parent;

    std::string m_local_port_s;
    Port *m_local_port = nullptr;

    std::shared_ptr<ParserNodeModuleWithPorts> m_peer;
    std::string m_peer_port_s;
    Port *m_peer_port = nullptr;

    void parse_peer(const std::string s_peer);

public:
    ParserNodeBinding(PlatformDescription &descr, const std::string &n,
                      ParserNodePlatform &root, ParserNodeModuleWithPorts &parent);
    virtual ~ParserNodeBinding();

    void second_pass();
    void instanciation_done();

    ParserNodeModuleWithPorts & get_peer() const;
    
    Port & get_local_port() const
    {
        assert(m_local_port != nullptr);
        return *m_local_port;
    }

    Port & get_peer_port() const
    {
        assert(m_peer_port != nullptr);
        return *m_peer_port;
    }
};

#endif
