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

#ifndef _UTILS_COMPONENT_PARAMETERS_H
#define _UTILS_COMPONENT_PARAMETERS_H

#include "parameter/parameter.h"
#include "rabbits/platform/description.h"
#include "rabbits/rabbits_exception.h"

class ComponentParameters {
public:
    typedef std::map<std::string, ParameterBase*>::iterator iterator;
    typedef std::map<std::string, ParameterBase*>::const_iterator const_iterator;

    class ParameterNotFoundException : public RabbitsException {
    protected:
        std::string make_what(const std::string &p) {
            return "Parameter `" + p + "` not found";
        }

    public:
        explicit ParameterNotFoundException(const std::string & c) : RabbitsException(make_what(c)) {}
        virtual ~ParameterNotFoundException() throw() {}
    };

protected:
    std::map<std::string, ParameterBase*> m_pool;
    PlatformDescription m_descr; /* The associated description */

public:
    ComponentParameters() {}
    ComponentParameters(const ComponentParameters &);

    virtual ~ComponentParameters() {
        iterator it;

        for (it = m_pool.begin(); it != m_pool.end(); it++) {
            delete it->second;
        }
    }

    void add(const std::string name, const ParameterBase &p) {
        m_pool[name] = p.clone();
    }

    void fill_from_description(const PlatformDescription &);

    bool exists(const std::string & name) {
        return m_pool.find(name) != m_pool.end();
    }

    ParameterBase& at(const std::string & name) {
        if (!exists(name)) {
            throw ParameterNotFoundException(name);
        }
        return *m_pool[name];
    }

    ParameterBase& operator[] (const std::string & name) {
        return at(name);
    }

    PlatformDescription& get_base_description() { return m_descr; }

    static ComponentParameters EMPTY;
};

#endif
