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

#ifndef _RABBITS_PLATFORM_PARSER_MODULE_H
#define _RABBITS_PLATFORM_PARSER_MODULE_H

#include "node.h"
#include "rabbits/module/namespace.h"
#include "rabbits/module/parameters.h"

class HasPortIface;

class ParserNodeModule : public ParserNode {
    std::string m_name;
    const Namespace &m_ns;
    std::string m_type;
    Parameters m_params;

public:
    ParserNodeModule(PlatformDescription &d, const std::string n,
                     ParserNodePlatform &root, const Namespace &ns);
    ParserNodeModule(const std::string name, const std::string &type,
                     const Parameters &params, ParserNodePlatform &root,
                     const Namespace &ns);
    ParserNodeModule(ParserNodePlatform &root, const Namespace &ns);
    virtual ~ParserNodeModule();

    const std::string & get_name() const;
    const std::string & get_type() const;
    const Namespace & get_namespace() const;
    const Parameters & get_params() const;

    template <class T>
    void add_field(const std::string &name, T& storage);
};

class ParserNodeModuleWithPorts : public ParserNodeModule {
private:
    HasPortIface * m_inst = nullptr;

protected:
    void set_inst(HasPortIface *inst) { m_inst = inst; }

public:
    ParserNodeModuleWithPorts(PlatformDescription &d, const std::string n,
                              ParserNodePlatform &root, const Namespace &ns);
    ParserNodeModuleWithPorts(const std::string &name, const std::string &type,
                              const Parameters &params, ParserNodePlatform &root,
                              const Namespace &ns);
    ParserNodeModuleWithPorts(HasPortIface *m, ParserNodePlatform &root,
                              const Namespace &ns);

    HasPortIface * get_inst() const { return m_inst; }
};

#endif
