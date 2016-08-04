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

#ifndef _RABBITS_PLATFORM_PARSER_H
#define _RABBITS_PLATFORM_PARSER_H

#include "description.h"

#include "parser/impl/node.h"
#include "parser/impl/platform.h"
#include "parser/impl/module.h"
#include "parser/impl/component.h"
#include "parser/impl/backend.h"
#include "parser/impl/plugin.h"
#include "parser/impl/binding.h"

class PlatformParser {
protected:
    std::string m_name;
    PlatformDescription m_descr;
    ParserNodePlatform m_root;

public:
    PlatformParser(const std::string &name, PlatformDescription &d,
                   ConfigManager &c)
        : m_name(name), m_descr(d), m_root(d, c)
    {
        m_root.second_pass();
    }

    PlatformParser(const std::string &name, ConfigManager &c)
        : m_name(name), m_root(c)
    {}

    virtual ~PlatformParser() {}

    const std::string& get_name() const { return m_name; }
    const PlatformDescription get_descr() const { return m_descr; }
    ParserNodePlatform & get_root() { return m_root; }

    void instanciation_done() { m_root.instanciation_done(); }
};

#endif
