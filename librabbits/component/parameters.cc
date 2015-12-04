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

#include "rabbits/component/parameters.h"

ComponentParameters ComponentParameters::EMPTY;

ComponentParameters::ComponentParameters(const ComponentParameters &p)
{
    const_iterator it;
    for(it = p.m_pool.begin(); it != p.m_pool.end(); it++) {
        add(it->first, *(it->second));
    }
}

void ComponentParameters::fill_from_description(const PlatformDescription &p)
{
    PlatformDescription::const_iterator it;
    m_descr = p;

    for (it = p.begin(); it != p.end(); it++) {
        if(exists(it->first)) {
            at(it->first).set(it->second);
        }
    }
}

