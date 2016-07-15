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

#ifndef _RABBITS_MODULE_FACTORY_H
#define _RABBITS_MODULE_FACTORY_H

#include <tuple>

#include "parameters.h"
#include "namespace.h"
#include "rabbits/config/has_config.h"


class ModuleFactoryBase : public HasParametersIface, public HasConfigIface
{
private:
    ConfigManager &m_config;
    std::string m_name;
    std::string m_description;
    const Namespace & m_namespace;
    Parameters m_params;

protected:
    template <class T>
    void add_param(const std::string &name, const T &t) { m_params.add(name, t); }

    ModuleFactoryBase(ConfigManager &config, const std::string & name,
                      const std::string & description, const Namespace & ns)
        : m_config(config), m_name(name), m_description(description), m_namespace(ns)
    {}

public:
    ModuleFactoryBase(const ModuleFactoryBase &) = delete;
    ModuleFactoryBase & operator= (const ModuleFactoryBase &) = delete;

    virtual ~ModuleFactoryBase() {}

    /**
     * @brief Return the name of the module.
     *
     * Return the name of the module associated to this factory.
     *
     * @return the name of the module.
     */
    const std::string & get_name() const { return m_name; }

    /**
     * @brief Return the description of the module.
     *
     * Return the description of the module associated to this factory.
     *
     * @return the description of the module.
     */
    const std::string & get_description() const { return m_description; }

    /**
     * @brief Return the namespace of the module.
     *
     * Return the namespace of the module associated to this factory.
     *
     * @return the namespace of the module.
     */
    const Namespace & get_namespace() const { return m_namespace; }

    /**
     * @brief Return the full name of the module.
     *
     * Return the full name of the module associated to this factory.
     * The full name is composed of the namespace name, a dot, and the module
     * name.
     *
     * @return the full name of the module.
     */
    std::string get_full_name() const { return m_namespace.get_name() + "." + m_name; }

    /* HasParametersIface */
    const Parameters & get_params() const { return m_params; }

    /* HasConfigIface */
    ConfigManager & get_config() const { return m_config; }
};

template <class Type>
class ModuleFactory : public ModuleFactoryBase {
protected:
    virtual Type * create(const std::string & name, Parameters & params) = 0;

    ModuleFactory(ConfigManager &config, const std::string & name, const std::string & description,
                  const Namespace & ns)
        : ModuleFactoryBase(config, name, description, ns) {}

    virtual ~ModuleFactory() {}

public:
    Type * create(const std::string & name, const PlatformDescription &p)
    {
        Parameters cp = get_params();
        cp.fill_from_description(p);
        cp.set_namespace(get_namespace());

        Type *t = create(name, cp);

        return t;
    }
};

#endif
