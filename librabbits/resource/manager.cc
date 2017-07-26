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

#include "rabbits/resource/manager.h"
#include "rabbits/logger.h"

#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using std::string;

ResourceManager::ResourceManager()
{}

ResourceManager::ResourceManager(const std::string &base_dir)
    : m_base(base_dir)
{}

ResourceManager::~ResourceManager()
{}

void ResourceManager::set_base_dir(const std::string &base_dir)
{
    m_base = base_dir;
}

ResourceInventory & ResourceManager::get_inventory(const std::string &dir)
{
    if (m_dirs.find(dir) == m_dirs.end()) {
        path p(m_base);
        p /= dir;

        if (!is_directory(p)) {
            LOG(APP, DBG) << "resource inventory " << p << " is not a directory\n";
            throw ResourceNotFoundException(dir, ResourceNotFoundException::INVENTORY);
        }

        m_dirs.emplace(dir, RMInventory(m_base, dir));
    }

    return m_dirs[dir];
}

Resource & ResourceManager::RMInventory::get_resource(const std::string &p_s)
{
    if (m_res.find(p_s) == m_res.end()) {
        path p(m_base);
        p /= m_inv;
        p /= p_s;

        if (!is_regular(p)) {
            LOG(APP, DBG) << "resource " << p << " is not a regular file\n";
            throw ResourceNotFoundException(p_s, ResourceNotFoundException::RESOURCE);
        }

        m_res.emplace(p_s, RMResource(m_base, m_inv, p_s));
    }

    return m_res[p_s];
}


std::string ResourceManager::RMInventory::get_absolute_path() const
{
    return (path(m_base) / path(m_inv)).string();
}

std::string ResourceManager::RMResource::get_absolute_path() const
{
    return (path(m_base) / path(m_inv) / path(m_path)).string();
}

std::string ResourceManager::RMResource::get_absolute_uri() const
{
    return string("file://") + get_absolute_path();
}
