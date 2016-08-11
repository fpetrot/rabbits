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

#ifndef _RABBITS_PLATFORM_PARSER_IMPL_NODE_H
#define _RABBITS_PLATFORM_PARSER_IMPL_NODE_H

#include <map>
#include <string>
#include <memory>
#include <cassert>

#include "../node.h"

inline ParserNode::ParserNode(PlatformDescription &descr, ParserNodePlatform &root)
    : m_descr(descr), m_root(root) {}

inline ParserNode::ParserNode(ParserNodePlatform &root)
    : m_root(root) {}

inline ParserNode::~ParserNode() {}

template <class T>
inline void ParserNode::add_optional_field(const std::string &name, const T& default_val, T& storage)
{
    if (!m_descr[name].is_scalar()) {
        storage = default_val;
    } else {
        try {
            storage = m_descr[name].as<T>();
        } catch(PlatformDescription::InvalidConversionException e) {
            throw InvalidFieldTypeException(m_descr[name], name, TypeId::get_typeid<T>());
        }
    }
}

template <class T>
inline void ParserNode::add_field(const std::string &name, T& storage)
{
    if (!m_descr[name].is_scalar()) {
        throw MissingFieldParseException(m_descr, name);
    }

    try {
        storage = m_descr[name].as<T>();
    } catch(PlatformDescription::InvalidConversionException e) {
        throw InvalidFieldTypeException(m_descr[name], name, TypeId::get_typeid<T>());
    }
}

template <class T, class... Args>
inline void ParserNode::add_named_subnodes(const std::string &name,
                                    NamedSubnodes<T> &storage,
                                    bool optional, Args&... args)
{
    if (!m_descr.exists(name)) {
        if (!optional) {
            throw MissingFieldParseException(m_descr, name);
        } else {
            return;
        }
    }

    if (!m_descr[name].is_map()) {
        throw PlatformParseException(m_descr[name], "Invalid node " + name);
    }

    for (auto &p: m_descr[name]) {
        storage[p.first] = std::make_shared<T>(p.second, p.first, get_root(), args...);
        add_subnode(storage[p.first]);
    }
}

template <class T, class... Args>
inline void ParserNode::add_named_subnodes(const std::string &name,
                                           NamedSubnodes<T> &storage, Args&... args)
{
    add_named_subnodes(name, storage, false, args...);
}

template <class T, class... Args>
inline void ParserNode::add_optional_named_subnodes(const std::string &name,
                                             NamedSubnodes<T> &storage, Args&... args)
{
    add_named_subnodes(name, storage, true, args...);
}

inline void ParserNode::add_subnode(std::shared_ptr<ParserNode> node)
{
    m_subnodes.insert(node);
}

inline void ParserNode::remove_subnode(std::shared_ptr<ParserNode> node)
{
    auto it = m_subnodes.find(node);

    assert(it != m_subnodes.end());
    m_subnodes.erase(it);
}

inline void ParserNode::second_pass()
{
    for (auto sub : m_subnodes) {
        sub->second_pass();
    }
}

inline void ParserNode::instanciation_done()
{
    for (auto sub : m_subnodes) {
        sub->instanciation_done();
    }
}

inline PlatformDescription ParserNode::get_descr() const
{
    return m_descr;
}

inline ParserNodePlatform & ParserNode::get_root()
{
    return m_root;
}

#endif
