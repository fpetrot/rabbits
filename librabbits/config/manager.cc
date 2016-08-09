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

#include <boost/filesystem.hpp>
#include <set>
#include <string>
#include <vector>

#include "rabbits/config.h"
#include "rabbits/config/manager.h"
#include "rabbits/logger.h"

using std::set;
using std::vector;
using std::string;
using namespace boost::filesystem;

ConfigManager * ConfigManager::m_config = nullptr;

ConfigManager::ConfigManager()
    : m_logger_app(*this)
    , m_logger_sim(*this)
    , m_global_params(Namespace::get(Namespace::GLOBAL))
    , m_dynloader(*this)
{
    add_global_param("config-dir",
                     Parameter<string>("Global configuration directory",
                                       RABBITS_CONFIG_PATH));

    add_global_param("selected-platform",
                     Parameter<string>("The selected platform",
                                       ""));

    add_global_param("color-output",
                     Parameter<bool>("Allow usage of colors when the "
                                     "output is a terminal",
                                     true));

    add_global_param("report-non-mapped-access",
                     Parameter<bool>("Report a simulation error when an initiator tries "
                                     "to access a memory address that lead to a "
                                     "non-mapped area on a bus.",
                                     true,
                                     true));
}

ConfigManager::~ConfigManager()
{
}

void ConfigManager::apply_aliases()
{
    for (auto alias : m_aliases) {
        const string &key = alias.first;
        ParameterBase &param = *alias.second;

        if (m_root_descr[key].is_scalar()) {
            param.set(m_root_descr[key]);
        }
    }
}

void ConfigManager::compute_platform(const string &name, const PlatformDescription &p)
{
    PlatformDescription platform = p;

    if (platform["inherit"].is_scalar()) {
        const string parent_name = platform["inherit"].as<string>();

        LOG(APP, DBG) << "Platform " << name << " inherits from `" << parent_name << "`\n";

        if (platform_exists(parent_name)) {
            PlatformDescription parent = get_platform(parent_name);

            platform = platform.merge(parent);
            platform.remove("generic");
        } else {
            LOG(APP, DBG) << "Platform " << name << " inherits from unknown platform `" << parent_name << "`\n";
        }
    }

    m_platforms[name] = platform;
}

void ConfigManager::recompute_platforms()
{
    m_platforms.clear();

    if (!m_root_descr["platforms"].is_map()) {
        LOG(APP, DBG) << "No platform found.\n";
        return;
    }

    for (auto &p : m_root_descr["platforms"]) {
        LOG(APP, DBG) << "Found platform " << p.first << "\n";
        compute_platform(p.first, p.second);
    }
}

void ConfigManager::load_config_from_description()
{
    if (!m_root_descr.exists("config")) {
        return;
    }

    PlatformDescription conf = m_root_descr["config"];
    set<string> to_load;

    if (conf.is_map()) {
        for (auto c: conf) {
            PlatformDescription & cc = c.second;
            if (cc.is_scalar()) {
                to_load.insert(cc.as<string>());
            }
        }
    } else if (conf.is_scalar()) {
        to_load.insert(conf.as<string>());
    }

    for (auto c: to_load) {
        add_config_file(c);
    }
}

void ConfigManager::recompute_config()
{
    m_root_descr = m_cmdline_descr.merge(m_config_file_descr);

    load_config_from_description();

    if (m_is_recomputing_config) {
        return;
    }

    m_is_recomputing_config = true;

    m_global_params.fill_from_description(m_root_descr["global"]);

    if (!m_platform_basename.empty()) {
        m_global_params["selected-platform"] = m_platform_basename;
    }

    apply_aliases();
    recompute_platforms();

    m_is_recomputing_config = false;
}

void ConfigManager::apply_description(PlatformDescription &d)
{
    m_config_file_descr = d.merge(m_config_file_descr);
    recompute_config();
}

void ConfigManager::parse_basename(const char *arg0)
{
    const string basename = path(arg0).filename().string();

    if (basename != RABBITS_APP_NAME) {
        LOG(APP, DBG) << "Trying to deduce selected platform from basename\n";
        const string prefix(RABBITS_PLATFORM_SYMLINK_PREFIX);

        if (basename.find(prefix) != 0) {
            LOG(APP, DBG) << "basename seems invalid. Giving up.\n";
        } else {
            m_platform_basename = basename.substr(prefix.size());
        }
    }
}

void ConfigManager::load_config_directory(path p)
{
    directory_iterator d;

    LOG(APP, DBG) << "Loading configuration files from " << p << "\n";

    if (!exists(p)) {
        LOG(APP, DBG) << "Directory " << p << " not found.\n";
        return;
    }

    if (!is_directory(p)) {
        LOG(APP, DBG) << p << " is not a directory.\n";
        return;
    }

    for (d = directory_iterator(p); d != directory_iterator(); d++) {
        directory_entry &dd = *d;

        if (is_directory(dd)) {
            load_config_directory(dd.path());
        } else if (is_regular_file(dd)) {
            add_config_file(dd.path().string());
        } else {
            LOG(APP, DBG) << "Skipping non-regular file " << dd.path() << "\n";
        }

    }
}

void ConfigManager::build_cmdline_unaries(set<string> &unaries) const
{
    /* We take every aliases that have boolean type */
    for (const auto &pair : m_aliases) {
        const string key = pair.first;
        const ParameterBase &p = *pair.second;

        if (p.is_convertible_to<bool>()) {
            unaries.insert(key);
        }
    }
}

void ConfigManager::add_cmdline(int argc, const char * const argv[])
{
    set<string> unaries;

    parse_basename(argv[0]);

    build_cmdline_unaries(unaries);
    m_cmdline_descr.parse_cmdline(argc, argv, unaries);

    recompute_config();

    path conf_dir(m_global_params["config-dir"].as<string>());
    load_config_directory(conf_dir);
}

bool ConfigManager::config_file_is_loaded(const std::string &path) const
{
    return m_loaded_config_files.find(path) != m_loaded_config_files.end();
}

void ConfigManager::add_yml_file(const string &filename)
{
    PlatformDescription d;

    LOG(APP, DBG) << "Loading YAML config file " << filename << "\n";

    if (config_file_is_loaded(filename)) {
        LOG(APP, DBG) << filename << " is already loaded. Skipping.\n";
        return;
    }

    try {
        d.load_file_yaml(filename);
    } catch (PlatformDescription::YamlParsingException e) {
        LOG(APP, ERR) << "Failed to parse YAML config file " << filename << ": " << e.what() << "\n";
    } catch (std::exception e) {
        LOG(APP, ERR) << "Failed to load YAML config file " << filename << ": " << e.what() << "\n";
        return;
    }

    m_loaded_config_files.insert(filename);

    apply_description(d);
}

void ConfigManager::add_yml(const string &yml_descr)
{
    PlatformDescription d;

    d.load_yaml(yml_descr);

    apply_description(d);
}

void ConfigManager::add_config_file(const std::string &path_s)
{
    path p(path_s);
    string ext = p.extension().string();

    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if ((ext == ".yml") || (ext == ".yaml")) {
        add_yml_file(path_s);
    } else {
        LOG(APP, DBG) << "Ignoring unknown file " << p << "\n";
    }
}

void ConfigManager::add_param_alias(const std::string &alias_key, ParameterBase &dest)
{
    m_aliases[alias_key] = &dest;
    apply_aliases();
}

void ConfigManager::add_global_param(const string &key, const ParameterBase &param)
{
    m_global_params.add(key, param);
}

Parameters & ConfigManager::get_global_params()
{
    return m_global_params;
}

void ConfigManager::get_dynlibs_to_load(vector<string> &dynlibs) const
{
    std::copy(m_dynlibs_to_load.begin(), m_dynlibs_to_load.end(), dynlibs.begin());
}

const ConfigManager::Platforms & ConfigManager::get_platforms() const
{
    return m_platforms;
}

PlatformDescription ConfigManager::get_platform(const string &name) const
{
    if (platform_exists(name)) {
        return m_platforms.at(name);
    } else {
        return PlatformDescription::INVALID_DESCRIPTION;
    }
}

PlatformDescription ConfigManager::apply_platform(const string &name)
{
    if (platform_exists(name)) {
        PlatformDescription p = get_platform(name);
        apply_description(p);
        return get_root_description();
    } else {
        return PlatformDescription::INVALID_DESCRIPTION;
    }
}
