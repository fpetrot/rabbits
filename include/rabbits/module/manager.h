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

#ifndef _RABBITS_MODULE_MANAGER_H
#define _RABBITS_MODULE_MANAGER_H

#include <map>
#include <memory>
#include <string>

#include "rabbits/logger.h"
#include "rabbits/rabbits_exception.h"
#include "factory.h"

/**
 * @brief Exception raised when a typed component has not been found.
 */
class FactoryNotFoundException : public RabbitsException {
protected:
    std::string make_what(std::string comp) { return "Factory `" + comp + "` not found."; }
public:
    explicit FactoryNotFoundException(const std::string & comp) : RabbitsException(make_what(comp)) {}
    virtual ~FactoryNotFoundException() throw() {}
};


class ModuleFactoryBase;

class ModuleManagerBase {
public:
    typedef std::shared_ptr<ModuleFactoryBase> Factory;
    typedef std::map<std::string, Factory > Factories;

    typedef typename Factories::iterator iterator;
    typedef typename Factories::const_iterator const_iterator;

private:
    std::map<std::string, std::shared_ptr<ModuleFactoryBase> > m_factories;

protected:
    void register_factory(Factory f)
    {
        const std::string &type = f->get_type();
        m_factories[type] = f;
    }
    
public:
    ModuleManagerBase() {}
    virtual ~ModuleManagerBase() {}


    bool type_exists(const std::string &type) const
    {
        return m_factories.find(type) != m_factories.end();
    }

    /**
     * @brief Find a module factory given its type
     *
     * @param type The module factory type.
     *
     * @return the module factory associated to the type.
     * @throw FactoryNotFoundException if the factory was not found.
     */
    Factory find_by_type(const std::string &type)
    {
        if (!type_exists(type)) {
            throw FactoryNotFoundException(type);
        }

        return m_factories[type];
    }

    /**
     * @brief Return an iterator to the first registered component factory.
     *
     * @return an iterator to the first registered component factory.
     */
    iterator begin() { return m_factories.begin(); }

    /**
     * @brief Return an iterator to the <i>past-the-end</i> registered component factory.
     *
     * @return an iterator to the <i>past-the-end</i> registered component factory.
     */
    iterator end() { return m_factories.end(); }

    /**
     * @brief Return an constant iterator to the first registered component factory.
     *
     * @return an iterator to the first registered component factory.
     */
    const_iterator begin() const { return m_factories.begin(); }

    /**
     * @brief Return an constant iterator to the <i>past-the-end</i> registered component factory.
     *
     * @return an iterator to the <i>past-the-end</i> registered component factory.
     */
    const_iterator end() const { return m_factories.end(); }

};


template <class TFactory>
class ModuleManager : public ModuleManagerBase {
public:
    typedef std::shared_ptr<TFactory> Factory;
    typedef std::map<std::string, Factory> Factories;

    typedef typename Factories::iterator iterator;
    typedef typename Factories::const_iterator const_iterator;

protected:
    Factories m_factories;

public:
    ModuleManager() {}
    ModuleManager(const ModuleManager &) = delete;
    ModuleManager & operator= (const ModuleManager&) = delete;

    virtual void register_factory(Factory f)
    {
        const std::string &type = f->get_type();
        m_factories[type] = f;

        ModuleManagerBase::register_factory(f);
        LOG(APP, DBG) << "Registering module " << f->get_full_name() << "\n";
    }

    bool type_exists(const std::string &type) const
    {
        return m_factories.find(type) != m_factories.end();
    }

    /**
     * @brief Find a module factory given its type
     *
     * @param type The module factory type.
     *
     * @return the module factory associated to the type.
     * @throw FactoryNotFoundException if the factory was not found.
     */
    Factory find_by_type(const std::string &type)
    {
        if (!type_exists(type)) {
            throw FactoryNotFoundException(type);
        }

        return m_factories[type];
    }

    /**
     * @brief Return an iterator to the first registered component factory.
     *
     * @return an iterator to the first registered component factory.
     */
    iterator begin() { return m_factories.begin(); }

    /**
     * @brief Return an iterator to the <i>past-the-end</i> registered component factory.
     *
     * @return an iterator to the <i>past-the-end</i> registered component factory.
     */
    iterator end() { return m_factories.end(); }

    /**
     * @brief Return an constant iterator to the first registered component factory.
     *
     * @return an iterator to the first registered component factory.
     */
    const_iterator begin() const { return m_factories.begin(); }

    /**
     * @brief Return an constant iterator to the <i>past-the-end</i> registered component factory.
     *
     * @return an iterator to the <i>past-the-end</i> registered component factory.
     */
    const_iterator end() const { return m_factories.end(); }
};

#endif
