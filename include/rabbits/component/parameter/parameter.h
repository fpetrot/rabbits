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

/**
 * @file parameter.h
 * @brief ParameterBase and Parameter class declaration
 */

#ifndef _UTILS_COMPONENT_PARAMETER_PARAMETER_H
#define _UTILS_COMPONENT_PARAMETER_PARAMETER_H

#include <string>
#include <sstream>

#include "param_data.h"

#include "rabbits/rabbits_exception.h"
#include "rabbits/platform/description.h"

/**
 * @brief Parameter, base class.
 *
 * This class allows parameter manipulation while being parameter type agnostic.
 * When a conversion is requested, a runtime check is performed to ensure the
 * parameter can be converted to the requested type.
 */
class ParameterBase {
public:
    /**
     * @brief Raised when converting a parameter to a type failed.
     */
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

    /**
     * @brief Convert a parameter to a given type.
     *
     * @tparam T type to convert the parameter to.
     *
     * @return The conversion result.
     *
     * @throw InvalidParameterTypeException if the conversion failed.
     */
    template <typename T> T as();

    /**
     * @brief Set the parameter to the value of the root node in the given PlatformDescription.
     *
     * The given PlatformDescription is supposed to be convertible to the type of the parameter.
     *
     * @param[in] p The platform description.
     */
    virtual void set(const PlatformDescription &p) = 0;

    /**
     * @brief Clone the parameter.
     *
     * The clone is dynamically allocated and must be deleted by the caller.
     *
     * @return a clone of the parameter.
     */
    virtual ParameterBase* clone() const = 0;

    /**
     * @brief Convert the parameter to string.
     *
     * This method is implemented using a std::stringstream for the conversion.
     * Thus, a function ostream& operator<<(ostream&, T) must exists for this method to
     * compile (with T being the parameter type).
     *
     * @return the string representing the parameter value.
     */
    virtual std::string to_str() const = 0;

    /**
     * @brief Return a textual description of the parameter.
     *
     * @return a textual description of the parameter.
     */
    const std::string & get_description() { return m_description; }
};

/**
 * @brief a Component parameter.
 *
 * Represent a component parameter having a value of type T, a description and
 * a compulsory default value.
 *
 * @tparam T the parameter type.
 */
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

    /**
     * @brief Get the value of the parameter.
     *
     * If the parameter is not set, this method returns its default value.
     *
     * @return the value of the parameter.
     */
    T get() const {
        if (m_data_storage.is_inited()) {
            return m_data_storage.get();
        } else {
            return m_default_storage.get();
        }
    }

    /**
     * @brief Set the value of the parameter.
     *
     * @param d the new value of the parameter.
     */
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
