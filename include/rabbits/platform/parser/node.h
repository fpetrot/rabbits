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

#ifndef _RABBITS_PLATFORM_PARSER_NODE_H
#define _RABBITS_PLATFORM_PARSER_NODE_H

#include <map>
#include <string>
#include <memory>

#include "rabbits/module/parameters.h"
#include "rabbits/datatypes/typeid.h"
#include "exceptions.h"

class ParserNodePlatform;

class ParserNode {
public:
    template <class SUBNODE>
    using NamedSubnodes = std::map< std::string, std::shared_ptr<SUBNODE> >;

private:
    PlatformDescription m_descr;
    ParserNodePlatform &m_root;

    std::vector< std::shared_ptr<ParserNode> > m_subnodes;

protected:
    template <class T>
    void add_optional_field(const std::string &name, const T& default_val, T& storage);
    template <class T>
    void add_field(const std::string &name, T& storage);
    template <class T, class... Args>
    void add_named_subnodes(const std::string &name,
                            NamedSubnodes<T> &storage,
                            bool optional = false,
                            Args&... args);
    template <class T, class... Args>
    void add_optional_named_subnodes(const std::string &name,
                            NamedSubnodes<T> &storage,
                            Args&... args);
public:
    ParserNode(PlatformDescription &descr, ParserNodePlatform &root);
    virtual ~ParserNode();

    virtual void second_pass();
    virtual void instanciation_done();

    PlatformDescription get_descr() const;
    ParserNodePlatform & get_root();
};


#endif
