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

#include "rabbits/dynloader/dynloader.h"
#include "rabbits/dynloader/dynlib.h"
#include "rabbits/config.h"
#include "rabbits/logger.h"
#include "rabbits/config/manager.h"


#include <sstream>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using std::string;
using std::vector;

class DynamicLoaderVisitor {
public:
    virtual bool visit(DynamicLoader &dyn, const path &p) = 0;
};

class RabbitsDynLibLoaderVisitor : public DynamicLoaderVisitor {
protected:
    int m_loaded;
public:
    RabbitsDynLibLoaderVisitor() : m_loaded(0) {}
    bool visit(DynamicLoader &dyn, const path &p);
    int get_num_loaded() { return m_loaded; }
};

class LibFinderVisitor : public DynamicLoaderVisitor {
protected:
    string m_name;
    bool m_found;
    string m_result;
public:
    LibFinderVisitor(const string n) : m_name(n), m_found(false) {}
    bool visit(DynamicLoader &dyn, const path &p);
    bool found() { return m_found; }
    string get_path() { return m_result; }
};


DynamicLoader::DynamicLoader(ConfigManager &config) : m_config(config)
{
#ifdef RABBITS_DYNLIB_SEARCH_PATH
    add_search_path(RABBITS_DYNLIB_SEARCH_PATH);
#endif
}

DynamicLoader::~DynamicLoader()
{
    std::map<string, DynLib*>::iterator it;
    rabbits_dynamic_unload_fn lib_unload;

    for (it = m_libs.begin(); it != m_libs.end(); it++) {
        LOG(APP, DBG) << "Unloading library " << it->first << "\n";
        lib_unload = (rabbits_dynamic_unload_fn)
            it->second->get_symbol(RABBITS_DYN_UNLOAD_SYM);
        lib_unload();

        DynLib::close(it->second);
    }
}

void DynamicLoader::add_search_path(const string &path)
{
    m_search_paths.push_back(path);
}

void DynamicLoader::add_colon_sep_search_paths(const string &paths)
{
    std::stringstream ss(paths);
    string p;

    while (std::getline(ss, p, ':')) {
        LOG(APP, DBG) << "Adding dynamic library search path " << p << "\n";
        add_search_path(p);
    }
}

void DynamicLoader::add_search_paths(const vector<string> &paths)
{
    vector<string>::const_iterator it;

    for (it = paths.begin(); it != paths.end(); it++) {
        m_search_paths.push_back(*it);
    }
}

bool RabbitsDynLibLoaderVisitor::visit(DynamicLoader &dyn, const path &p)
{
    if(dyn.load_rabbits_dynlib(p.string())) {
        m_loaded++;
    }

    return true;
}

bool LibFinderVisitor::visit(DynamicLoader &dyn, const path &p)
{
    if(p.stem() == m_name) {
        m_found = true;
        m_result = p.string();
        return false;
    }

    return true;
}

void DynamicLoader::search_libs_and_visit(DynamicLoaderVisitor &v)
{
    vector<string>::iterator it;

    for (it = m_search_paths.begin(); it != m_search_paths.end(); it++) {
        try {
            path p(*it);

            if (!exists(p)) {
                LOG(APP, DBG) << "Directory " << *it << " not found.\n";
                continue;
            }

            if (!is_directory(p)) {
                LOG(APP, DBG) << *it << " is not a directory.\n";
                continue;
            }

            directory_iterator dir_it;

            for (dir_it = directory_iterator(p); dir_it != directory_iterator(); dir_it++) {
                directory_entry &entry = *dir_it;

                if (!is_regular_file(entry)) {
                    continue;
                }

                if (entry.path().extension().string() == "." + DynLib::get_lib_extension()) {
                    LOG(APP, DBG) << "Found " << entry << "\n";
                    if(!v.visit(*this, entry.path())) {
                        return;
                    }
                }
            }
        } catch(const filesystem_error &e) {
            LOG(APP, DBG) << e.what() << "\n";
            LOG(APP, DBG) << "Skipping " << *it << "\n";
        }
    }
}

int DynamicLoader::search_and_load_rabbits_dynlibs()
{
    RabbitsDynLibLoaderVisitor v;
    search_libs_and_visit(v);

    return v.get_num_loaded();
}

DynLib * DynamicLoader::load_library(const std::string &path)
{
    return DynLib::open(path);
}

DynLib * DynamicLoader::search_and_load_library(const std::string &filename)
{
    DynLib *ret = NULL;
    LibFinderVisitor v(filename);
    search_libs_and_visit(v);

    if (!v.found()) {
        return NULL;
    }

    ret = DynLib::open(v.get_path());

    return ret;
}

bool DynamicLoader::load_rabbits_dynlib(const string &filename)
{
    DynLib *l = NULL;
    rabbits_dynamic_api_version_fn lib_api_version = NULL;
    rabbits_dynamic_info_fn lib_info = NULL;
    rabbits_dynamic_load_fn lib_load = NULL;
    const RabbitsDynamicInfo *info = NULL;

    if (m_libs.find(filename) != m_libs.end()) {
        LOG(APP, DBG) << filename << " already loaded. Skipping\n";
        return true;
    }

    try {
        l = DynLib::open(filename);
    } catch (DynLib::CannotOpenDynLibException e) {
        LOG(APP, DBG) << e.what() << "\n";
        LOG(APP, DBG) << "skipping dynamic library " << filename << "\n";
        return false;
    }


    if ((!l->check_symbol(RABBITS_DYN_API_VER_SYM))
        || (!l->check_symbol(RABBITS_DYN_INFO_SYM))
        || (!l->check_symbol(RABBITS_DYN_LOAD_SYM))
        || (!l->check_symbol(RABBITS_DYN_UNLOAD_SYM))) {
        LOG(APP, DBG) << "skipping dynamic library " << filename << ": doesn't seem to be Rabbits compatible\n";
        DynLib::close(l);
        return false;
    }

    lib_api_version = (rabbits_dynamic_api_version_fn) l->get_symbol(RABBITS_DYN_API_VER_SYM);

    if (lib_api_version() != RABBITS_API_VERSION) {
        LOG(APP, WRN) << "Unable to load dynamic library " << filename << ": API version mismatch\n";
        LOG(APP, WRN) << "Need: " << RABBITS_API_VERSION << ", got: " << lib_api_version() << "\n";
        DynLib::close(l);
        return false;
    }

    m_libs[filename] = l;

    lib_info = (rabbits_dynamic_info_fn) l->get_symbol(RABBITS_DYN_INFO_SYM);

    info = lib_info();

    LOG(APP, DBG) <<  "Loading dynamic library `" << info->name
                  << "' ver. " << info->version_str << "\n";

    lib_load = (rabbits_dynamic_load_fn) l->get_symbol(RABBITS_DYN_LOAD_SYM);
    lib_load(m_config);

    LOG(APP, DBG) << "Loaded dynamic library " << filename << "\n";

    return true;
}
