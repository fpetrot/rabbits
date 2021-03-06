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
#include "rabbits/module/manager.h"

class HasPortIface;
class ParserNodeBinding;

class ParserNodeModule : public ParserNode {
    std::string m_name;
    const Namespace &m_ns;
    std::string m_type;
    Parameters m_params;
    bool m_params_is_set = false;
    ModuleManagerBase::Factory m_factory;

protected:
    virtual ModuleManagerBase::Factory get_module_factory();
    void set_params(const Parameters &p);

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
    ModuleManagerBase::Factory get_factory();
    const Parameters & get_params();

    template <class T>
    void add_field(const std::string &name, T& storage);
};

class ParserNodeModuleWithPorts : public ParserNodeModule {
private:
    HasPortIface * m_inst = nullptr;
    NamedSubnodes<ParserNodeBinding> m_bindings;

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

    NamedSubnodes<ParserNodeBinding> & get_bindings() { return m_bindings; }

    bool binding_exists(const std::string &port) const;

    void add_binding(const std::string local_port,
                     std::shared_ptr<ParserNodeModuleWithPorts> peer,
                     const std::string &peer_port,
                     PlatformDescription &params);

    void remove_binding_if_exists(const std::string local_port);
};

#endif
