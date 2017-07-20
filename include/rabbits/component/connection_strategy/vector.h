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

#ifndef _RABBITS_COMPONENT_CONNECTION_STRATEGY_VECTOR_H
#define _RABBITS_COMPONENT_CONNECTION_STRATEGY_VECTOR_H

#include "rabbits/component/connection_strategy.h"

template <class T>
class VectorPortBase;

template <typename T>
class VectorCS : public ConnectionStrategy< VectorCS<T> > {
public:
    using typename ConnectionStrategyBase::BindingResult;

private:
    VectorPortBase<T> &m_vec;

public:
    VectorCS(VectorPortBase<T> &v) : m_vec(v) {}
    virtual ~VectorCS() {}

    BindingResult bind_peer(VectorCS<T> &cs, PlatformDescription &d);
    BindingResult bind_hierarchical(VectorCS<T> &parent_cs);

    virtual const char * get_typeid() const { return "vector"; }
};

#endif
