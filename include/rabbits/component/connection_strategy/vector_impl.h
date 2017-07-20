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

#ifndef _RABBITS_COMPONENT_CONNECTION_STRATEGY_VECTOR_IMPL_H
#define _RABBITS_COMPONENT_CONNECTION_STRATEGY_VECTOR_IMPL_H

#include "vector.h"
#include "rabbits/component/port/vector.h"

template <class T>
typename VectorCS<T>::BindingResult VectorCS<T>::bind_peer(VectorCS<T> &cs, ConnectionInfo &info, PlatformDescription &d)
{
    /* Size compatibility */
    if (m_vec.size() != cs.m_vec.size()) {
        LOG(APP, ERR) << "Vector port size mismatch while binding " 
                   << m_vec.full_name() << " (size: " << m_vec.size() << ") to "
                   << cs.m_vec.full_name() << " (size: " << cs.m_vec.size() << "\n";
        return BindingResult::BINDING_ERROR;
    }

    for (unsigned int i = 0; i < m_vec.size(); i++) {
        T & p0 = m_vec[i];
        T & p1 = cs.m_vec[i];

        if (!p0.connect(p1, d)) {
            LOG(APP, WRN) << "Vector element " << p0.full_name()
                << " is not connectable to " << p1.full_name();
        }
    }

    return BindingResult::BINDING_OK;
}

template <class T>
typename VectorCS<T>::BindingResult VectorCS<T>::bind_hierarchical(VectorCS<T> &cs, ConnectionInfo &info)
{
    /* Size compatibility */
    if (m_vec.size() != cs.m_vec.size()) {
        LOG(APP, ERR) << "Vector port size mismatch while binding "
            << m_vec.full_name() << " (size: " << m_vec.size() << ") to "
            << cs.m_vec.full_name() << " (size: " << cs.m_vec.size() << "\n";
        return BindingResult::BINDING_ERROR;
    }

    for (unsigned int i = 0; i < m_vec.size(); i++) {
        T & p0 = m_vec[i];
        T & p1 = cs.m_vec[i];

        p0(p1);
    }

    return BindingResult::BINDING_OK;
}

#endif
