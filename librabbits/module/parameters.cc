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

#include "rabbits/module/parameters.h"

Parameters Parameters::EMPTY;

Parameters::Parameters(const Parameters &p)
{
    m_descr = p.m_descr;
    m_namespace = p.m_namespace;
    m_module = p.m_module;

    for (auto param : p.m_pool) {
        add(param.first, *param.second);
    }
}

Parameters& Parameters::operator= (const Parameters& p)
{
    m_descr = p.m_descr;
    m_namespace = p.m_namespace;
    m_module = p.m_module;

    for (auto param : p.m_pool) {
        add(param.first, *param.second);
    }

    return *this;
}

void Parameters::fill_from_description(const PlatformDescription &p)
{
    if (&p != &m_descr) {
        m_descr = p;
    }

    if (!p.is_map()) {
        return;
    }

    for (auto param: p) {
        if(exists(param.first)) {
            at(param.first).set(param.second);
        }
    }
}
