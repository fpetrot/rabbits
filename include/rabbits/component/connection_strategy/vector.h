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
#include "rabbits/component/port.h"

class VectorCS : public ConnectionStrategy< VectorCS > {
public:
    using typename ConnectionStrategyBase::BindingResult;
    using typename ConnectionStrategyBase::ConnectionInfo;

    typedef std::vector< Port* > PortCollection;

private:
    PortCollection &m_vec;

public:
    VectorCS(PortCollection &v) : m_vec(v) {}
    virtual ~VectorCS() {}

    BindingResult bind_peer(VectorCS &cs, ConnectionInfo &info, PlatformDescription &d)
    {
        /* Size compatibility */
        if (m_vec.size() != cs.m_vec.size()) {
            LOG(APP, ERR) << "Vector port size mismatch\n";
            return BindingResult::BINDING_ERROR;
        }

        for (unsigned int i = 0; i < m_vec.size(); i++) {
            Port* p0 = m_vec[i];
            Port* p1 = cs.m_vec[i];

            if (!p0->connect(*p1, d)) {
                LOG(APP, WRN) << "Vector element " << p0->full_name()
                    << " is not connectable to " << p1->full_name();
            }
        }

        return BindingResult::BINDING_OK;
    }

    BindingResult bind_hierarchical(VectorCS &parent_cs, ConnectionInfo &info)
    {
        /* Size compatibility */
        if (m_vec.size() != parent_cs.m_vec.size()) {
            LOG(APP, ERR) << "Vector port size mismatch\n";
            return BindingResult::BINDING_ERROR;
        }

        for (unsigned int i = 0; i < m_vec.size(); i++) {
            Port* p0 = m_vec[i];
            Port* p1 = parent_cs.m_vec[i];

            (*p0)(*p1);
        }

        return BindingResult::BINDING_OK;
    }

    virtual const char * get_typeid() const { return "vector"; }
};

#endif
