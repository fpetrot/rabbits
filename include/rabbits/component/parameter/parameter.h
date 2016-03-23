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

#ifndef _UTILS_COMPONENT_PARAMETER_PARAMETER_H
#define _UTILS_COMPONENT_PARAMETER_PARAMETER_H

#include <string>
#include <sstream>

#include "param_data.h"

#include "rabbits/rabbits_exception.h"
#include "rabbits/platform/description.h"

class ParameterBase {
public:
    class InvalidParameterTypeException : public RabbitsException {
    public:
        explicit InvalidParameterTypeException(const std::string & what) : RabbitsException(what) {}
        virtual ~InvalidParameterTypeException() throw() {}
    };

private:
    std::string m_description;

    ParamDataBase *m_data;
    ParamDataBase *m_default;

protected:
    ParameterBase(const std::string & descr, ParamDataBase *data, ParamDataBase *default_value)
        : m_description(descr), m_data(data), m_default(default_value) {}
    ParameterBase(const ParameterBase &p, ParamDataBase *data, ParamDataBase *default_value)
        : m_description(p.m_description), m_data(data), m_default(default_value) {}

public:
    virtual ~ParameterBase() {}

    template <typename T> T as();
    virtual void set(const PlatformDescription &) = 0;
    virtual ParameterBase* clone() const = 0;
    virtual std::string to_str() const = 0;

    const std::string & get_description() { return m_description; }
};

template <typename T>
class Parameter : public ParameterBase {
protected:
    ParamData<T> m_data_storage;
    ParamData<T> m_default_storage;

public:
    Parameter(const std::string & description, const T &default_value)
        : ParameterBase(description, &m_data_storage, &m_default_storage)
        , m_default_storage(default_value) {}
    Parameter(const Parameter &p)
        : ParameterBase(p, &m_data_storage, &m_default_storage)
        , m_data_storage(p.m_data_storage)
        , m_default_storage(p.m_default_storage) {}
    virtual ~Parameter() {}

    T get() const {
        if (m_data_storage.is_inited()) {
            return m_data_storage.get();
        } else {
            return m_default_storage.get();
        }
    }

    void set(const T& d) {
        m_data_storage.set(d);
    }

    virtual void set(const PlatformDescription &p) {
        m_data_storage.set(p.as<T>());
    }

    virtual ParameterBase* clone() const { return new Parameter<T>(*this); }

    virtual std::string to_str() const {
        std::stringstream ss;
        ss << get();
        return ss.str();
    }
};

template <typename T>
inline T ParameterBase::as() {
    Parameter<T> *p = dynamic_cast<Parameter<T>*>(this);
    if (p == NULL) {
        throw InvalidParameterTypeException("Parameter type mismatch");
    }
    return p->get();
}

#endif
