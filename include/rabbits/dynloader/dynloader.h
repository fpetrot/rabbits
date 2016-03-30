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
 * @file dynloader.h
 * @brief DynamicLoader class declaration.
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

/**
 * @brief Rabbits dynamic library loader.
 *
 * This class load dynamic libraries and Rabbits dynlibs.
 */
class DynamicLoader {
private:
    static DynamicLoader *m_inst;
    DynamicLoader();

protected:
    std::map<std::string, DynLib*> m_libs;
    std::vector<std::string> m_search_paths;

    void search_libs_and_visit(DynamicLoaderVisitor &);

public:
    /**
     * @brief Return the singleton instance of the DynamicLoader.
     *
     * @return the singleton instance of the DynamicLoader
     */
    static DynamicLoader& get() {
        if (m_inst == NULL) {
            m_inst = new DynamicLoader;
        }
        return *m_inst;
    }

    virtual ~DynamicLoader();

    /**
     * @brief Add a search path for the dynamic libraries.
     *
     * @param[in] path The path to add.
     */
    void add_search_path(const std::string &path);

    /**
     * @brief Add a set of search paths separated by a colon.
     *
     * @param[in] paths The paths to add.
     */
    void add_colon_sep_search_paths(const std::string &paths);

    /**
     * @brief Add a set of search paths.
     *
     * @param[in] paths the paths to add.
     */
    void add_search_paths(const std::vector<std::string> &paths);

    /**
     * @brief Search and load all the Rabbits dynlibs in the search paths.
     *
     * @return The number of loaded Rabbits dynlibs.
     */
    int search_and_load_rabbits_dynlibs();

    /**
     * @brief Load a Rabbits dynlib.
     *
     * @param[in] filename Path to the Rabbits dynlib.
     *
     * @return true if the loading was successful, false otherwise.
     */
    bool load_rabbits_dynlib(const std::string &filename);

    /**
     * @brief Load and return a dynamic library.
     *
     * @param[in] path Path to the dynamic library
     *
     * @return the loaded library, NULL on error.
     */
    DynLib * load_library(const std::string &path);

    /**
     * @brief Search, load and return a dynamic library.
     *
     * The search is performed in the search path.
     *
     * @param[in] filename Name of the dynamic library
     *
     * @return the loaded library, NULL on error.
     */
    DynLib * search_and_load_library(const std::string &filename);
};

#endif
