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

#ifndef _RABBITS_PLATFORM_PARSER_COMPONENT_H
#define _RABBITS_PLATFORM_PARSER_COMPONENT_H

#include "module.h"

class ParserNodeBinding;
class ComponentBase;

class ParserNodeComponent : public ParserNodeModuleWithPorts {
protected:
    ComponentBase *m_inst = nullptr;
    std::string m_implem;
    NamedSubnodes<ParserNodeBinding> m_bindings;

public:
    ParserNodeComponent(PlatformDescription &descr, const std::string &n, ParserNodePlatform &root);
    virtual ~ParserNodeComponent();

    bool implem_is_set() const;
    const std::string & get_implem() const;

    void set_inst(ComponentBase *inst);
    ComponentBase *get_inst() const { return m_inst; }

    NamedSubnodes<ParserNodeBinding> & get_bindings() { return m_bindings; }
};

#endif
