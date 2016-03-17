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

#ifndef _DYNLOADER_DYNLOADER_H
#define _DYNLOADER_DYNLOADER_H

#include <vector>
#include <string>
#include <map>

extern "C" {

struct RabbitsDynamicInfo {
    const char *name;
    const char *version_str;
};

typedef int (*rabbits_dynamic_api_version_fn)(void);
typedef const RabbitsDynamicInfo * (*rabbits_dynamic_info_fn)(void);
typedef void (*rabbits_dynamic_load_fn)(void);
typedef void (*rabbits_dynamic_unload_fn)(void);
}

#define RABBITS_DYN_API_VER_SYM "rabbits_dynamic_api_version"
#define RABBITS_DYN_INFO_SYM "rabbits_dynamic_info"
#define RABBITS_DYN_LOAD_SYM "rabbits_dynamic_load"
#define RABBITS_DYN_UNLOAD_SYM "rabbits_dynamic_unload"

class DynLib;
class DynamicLoaderVisitor;

class DynamicLoader {
private:
    static DynamicLoader *m_inst;
    DynamicLoader();

protected:
    std::map<std::string, DynLib*> m_libs;
    std::vector<std::string> m_search_paths;

    void search_libs_and_visit(DynamicLoaderVisitor &);

public:
    static DynamicLoader& get() {
        if (m_inst == NULL) {
            m_inst = new DynamicLoader;
        }
        return *m_inst;
    }

    virtual ~DynamicLoader();

    void add_search_path(const std::string &path);
    void add_colon_sep_search_paths(const std::string &paths);
    void add_search_paths(const std::vector<std::string> &paths);

    int search_and_load_rabbits_dynlibs();
    bool load_rabbits_dynlib(const std::string &filename);

    DynLib * load_library(const std::string &path);
    DynLib * search_and_load_library(const std::string &filename);
};

#endif
