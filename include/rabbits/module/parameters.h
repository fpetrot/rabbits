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
 * @file parameters.h
 * @brief Parameters class declaration.
 */

#ifndef _UTILS_COMPONENT_PARAMETERS_H
#define _UTILS_COMPONENT_PARAMETERS_H

#include "parameter/parameter.h"
#include "rabbits/platform/description.h"
#include "rabbits/rabbits_exception.h"

/**
 * @brief Component parameters collection.
 */
class Parameters {
public:
    typedef std::map<std::string, ParameterBase*>::iterator iterator;
    typedef std::map<std::string, ParameterBase*>::const_iterator const_iterator;

    /**
     * @brief Raised when the requested parameter was not found.
     */
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
    std::string m_namespace;

public:
    Parameters() {}
    Parameters(const std::string &params_namespace) : m_namespace(params_namespace) {}
    Parameters(const Parameters &);

    virtual ~Parameters() {
        iterator it;

        for (it = m_pool.begin(); it != m_pool.end(); it++) {
            delete it->second;
        }
    }

    /**
     * @brief Add a parameter to the collection
     *
     * @param[in] name Name of the parameter.
     * @param[in] p the parameter to add.
     */
    void add(const std::string name, const ParameterBase &p) {
        m_pool[name] = p.clone();
        m_pool[name]->set_name(name);
        m_pool[name]->set_namespace(m_namespace);
    }

    void set_namespace(const std::string &_namespace) {
        m_namespace = _namespace;

        for (auto c: *this) {
            c.second->set_namespace(m_namespace);
        }
    }

    /**
     * @brief Fill the collection from the given PlatformDescription.
     *
     * @param[in] d The platform description to fill the collection from.
     */
    void fill_from_description(const PlatformDescription &d);

    /**
     * @brief Check if a parameter exists in the collection.
     *
     * @param name The name of the parameter.
     *
     * @return true if the parameter exists, false otherwise.
     */
    bool exists(const std::string & name) const {
        return m_pool.find(name) != m_pool.end();
    }

    /**
     * @brief Return true if the collection is empty.
     *
     * @return true if the collection is empty, false otherwise.
     */
    bool empty() const { return m_pool.size() == 0; }

    /**
     * @brief Return the parameter associated with the given name.
     *
     * @param[in] name the name of the parameter.
     *
     * @return the parameter.
     * @throw ParameterNotFoundException if the parameter does not exists.
     */
    ParameterBase& at(const std::string & name) {
        if (!exists(name)) {
            throw ParameterNotFoundException(name);
        }
        return *m_pool[name];
    }

    /**
     * @brief Return the parameter associated with the given name.
     *
     * @param[in] name the name of the parameter.
     *
     * @return the parameter.
     * @throw ParameterNotFoundException if the parameter does not exists.
     */
    ParameterBase& operator[] (const std::string & name) {
        return at(name);
    }

    /**
     * @brief Return the description used to fill this collection.
     *
     * @return the description used to fill this description.
     */
    PlatformDescription& get_base_description() { return m_descr; }

    /**
     * @brief Return an iterator to the first parameter of the collection.
     *
     * @return an iterator to the first parameter of the collection.
     */
    iterator begin() { return m_pool.begin(); }

    /**
     * @brief Return an iterator to the <i>past-the-end</i> parameter of the collection.
     *
     * @return an iterator to the <i>past-the-end</i> parameter of the collection.
     */
    iterator end() { return m_pool.end(); }

    /**
     * @brief Return a constant iterator to the first parameter of the collection.
     *
     * @return an constant iterator to the first parameter of the collection.
     */
    const_iterator begin() const { return m_pool.begin(); }

    /**
     * @brief Return a constant iterator to the <i>past-the-end</i> parameter of the collection.
     *
     * @return a constant iterator to the <i>past-the-end</i> parameter of the collection.
     */
    const_iterator end() const { return m_pool.end(); }

    /**
     * @brief The empty parameters collection.
     */
    static Parameters EMPTY;
};

#endif
