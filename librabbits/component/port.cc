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

#include "rabbits/component/port.h"
#include "rabbits/component/component.h"

using namespace sc_core;

void Port::declare_parent(HasPortIface *p)
{
    m_parent = p;

    if (p) {
        m_parent->declare_port(*this, m_name);
    }
}

void Port::declare_parent(sc_object *p)
{
    HasPortIface *pp = dynamic_cast<HasPortIface*>(p);

    if (pp == NULL) {
        return;
    }

    declare_parent(pp);
}

void Port::add_attr_to_parent(const std::string & key, const std::string & value)
{
    HasAttributesIface *parent = dynamic_cast<HasAttributesIface*>(m_parent);

    if (parent != NULL) {
        parent->add_attr(key, value);
    }
}
