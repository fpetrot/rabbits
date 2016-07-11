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

#ifndef _RABBITS_CONFIG_MANAGER_H
#define _RABBITS_CONFIG_MANAGER_H

#include <string>
#include <vector>
#include <map>

#include "rabbits/module/parameters.h"
#include "rabbits/platform/description.h"

namespace boost {
    namespace filesystem {
        class path;
    }
}

class ConfigManager {
public:
    typedef std::map<std::string, PlatformDescription> Platforms;
    typedef std::map<std::string, ParameterBase*> ParamAliases;

private:
    static ConfigManager *m_manager;

    PlatformDescription m_root_descr;
    PlatformDescription m_cmdline_descr;
    PlatformDescription m_config_file_descr;

    Platforms m_platforms;

    Parameters m_global_params;

    std::vector<std::string> m_dynlibs_to_load;
    std::set<std::string> m_loaded_config_files;
    ParamAliases m_aliases;

    std::string m_platform_basename;

    bool m_is_recomputing_config = false;

    void apply_description(PlatformDescription &d);
    void parse_basename(const char *arg0);
    void load_config_directory(boost::filesystem::path p);
    void load_config_from_description();

    void compute_platform(const std::string &name, const PlatformDescription &p);
    void recompute_platforms();
    void apply_aliases();
    void recompute_config();

    void build_cmdline_unaries(std::set<std::string> &unaries) const;

public:
    static void set_manager(ConfigManager &c) { m_manager = &c; }
    static ConfigManager & get_manager() { return *m_manager; }

    ConfigManager();
    virtual ~ConfigManager();

    void add_cmdline(int argc, const char * const argv[]);
    void add_yml_file(const std::string &filename);
    void add_yml(const std::string &yml_descr);
    void add_config_file(const std::string &path);

    bool config_file_is_loaded(const std::string &path) const;

    void add_param_alias(const std::string &alias_key, ParameterBase &dest);
    const ParamAliases & get_param_aliases() const { return m_aliases; }
    void add_global_param(const std::string &key, const ParameterBase &param);
    Parameters & get_global_params();
    
    void get_dynlibs_to_load(std::vector<std::string> &dynlibs) const;

    bool platform_exists(const std::string &name) const { return m_platforms.find(name) != m_platforms.end(); }
    PlatformDescription get_platform(const std::string &name) const;
    PlatformDescription apply_platform(const std::string &name);

    PlatformDescription get_root_description() const { return m_root_descr; }
};

#endif
