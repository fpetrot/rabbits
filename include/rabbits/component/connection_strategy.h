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

#ifndef _RABBITS_COMPONENT_CONNECTION_STRATEGY_H
#define _RABBITS_COMPONENT_CONNECTION_STRATEGY_H

#include <cstdlib>
#include "rabbits/logger.h"
#include "rabbits/platform/description.h"

class ConnectionStrategyBase {
public:
    enum BindingType {
        PEER, HIERARCHICAL
    };

    enum BindingResult {
        BINDING_OK,
        BINDING_TRY_NEXT,
        BINDING_ERROR,
        BINDING_HIERARCHICAL_TYPE_MISMATCH
    };

    virtual ~ConnectionStrategyBase() {}
    virtual bool is_compatible_with(const ConnectionStrategyBase &cs) const = 0;

    virtual BindingResult bind(ConnectionStrategyBase &cs, BindingType t,
                      PlatformDescription &d = PlatformDescription::INVALID_DESCRIPTION) = 0;

    virtual const char * get_typeid() const { return "?"; }
};

template <typename T>
class ConnectionStrategy : public ConnectionStrategyBase {
public:
    virtual ~ConnectionStrategy() {}

    bool is_compatible_with(const ConnectionStrategyBase &cs) const {
        return (dynamic_cast<const T*>(&cs)) != nullptr;
    }

    BindingResult bind(ConnectionStrategyBase &cs, BindingType t,
                       PlatformDescription &d = PlatformDescription::INVALID_DESCRIPTION)
    {
        if (!is_compatible_with(cs)) {
            LOG(APP, ERR) << "Incompatible strategies bind attempt\n";
            abort();
        }

        T &cs0 = static_cast< T& >(*this);
        T &ccs = static_cast< T& >(cs);

        switch (t) {
        case PEER:
            return cs0.bind_peer(ccs, d);

        case HIERARCHICAL:
            return cs0.bind_hierarchical(ccs);
        }

        /* Quiet, compiler */
        return BINDING_ERROR;
    }
};

#endif
