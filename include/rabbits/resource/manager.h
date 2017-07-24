/*
 *  This file is part of Rabbits
 *  Copyright (C) 2017  Clement Deschamps and Luc Michel
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
#pragma once

#include <string>
#include <map>

#include "rabbits/rabbits_exception.h"

class ResourceNotFoundException : public RabbitsException {
public:
    enum Kind { RESOURCE, INVENTORY };

protected:
    Kind m_kind;

    std::string make_what(const std::string &path, Kind k)
    {
        std::string ret = "Resource";

        if (k == INVENTORY) {
            ret += " inventory";
        }

        ret += " `";
        ret += path;
        ret += "` not found";

        return ret;
    }

public:
    ResourceNotFoundException(const std::string & path, Kind k)
        : RabbitsException(make_what(path, k)), m_kind(k) {}

    Kind get_kind() { return m_kind; }
};

class Resource {
public:
    virtual std::string get_absolute_path() const = 0;
    virtual std::string get_absolute_uri() const = 0;
};

class ResourceInventory {
public:
    virtual Resource & get_resource(const std::string &path) = 0;
    virtual std::string get_absolute_path() const = 0;
};

class ResourceManager {
private:
    class RMResource : public Resource {
    private:
        std::string m_base;
        std::string m_inv;
        std::string m_path;

    public:
        RMResource() {}
        RMResource(const std::string &base, const std::string &inv, const std::string &path)
            : m_base(base), m_inv(inv), m_path(path) {}

        std::string get_absolute_path() const;
        std::string get_absolute_uri() const;
    };

    class RMInventory : public ResourceInventory {
    private:
        const std::string m_base;
        std::string m_inv;
        std::map<std::string, RMResource> m_res;

    public:
        RMInventory() {}
        RMInventory(const std::string &base, const std::string &inv)
            : m_base(base), m_inv(inv) {}

        Resource & get_resource(const std::string &path);
        std::string get_absolute_path() const;
    };

    std::string m_base;

    std::map<std::string, RMInventory> m_dirs;

public:
    ResourceManager();
    ResourceManager(const std::string &base_dir);
    virtual ~ResourceManager();

    void set_base_dir(const std::string &base_dir);
    ResourceInventory & get_inventory(const std::string &inv);
};
