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
#include <systemc>
#include <list>

#include "rabbits/config.h"
#include "rabbits/config/manager.h"
#include "rabbits/logger.h"
#include "rabbits/ui/ui.h"

using std::set;
using std::vector;
using std::string;
using std::pair;
using std::list;

using namespace boost::filesystem;

ConfigManager * ConfigManager::m_config = nullptr;

ConfigManager::ConfigManager()
    : m_global_params(Namespace::get(Namespace::GLOBAL))
    , m_root_loggers(m_global_params, *this)
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

    add_global_param("log-target",
                     Parameter<string>("Specify the log target (valid options "
                                       "are `stdout', `stderr' and `file')",
                                       "stderr"));

    add_global_param("log-file",
                     Parameter<string>("Specify the log file",
                                       "rabbits.log"));

    add_global_param("log-level",
                     Parameter<string>("Specify the log level (valid options "
                                       "are `trace', `debug', `info', `warning', `error')",
                                       "info"));

    add_global_param("debug",
                     Parameter<bool>("Set log level to `debug' "
                                     "(equivalent to `-global.log-level debug')",
                                     false));

    add_global_param("trace",
                     Parameter<bool>("Set log level to `trace' "
                                     "(equivalent to `-global.log-level trace')",
                                     false));

    Logger &sim = m_root_loggers.get_logger(LogContext::SIM);
    sim.set_custom_banner([] (Logger &l, const std::string &banner) {
        l << format::purple << "[sim]";
        if (sc_core::sc_get_status() == sc_core::SC_ELABORATION) {
            l << format::green << "[elaboration]" << format::reset;
        } else {
            l << format::green << "[" << sc_core::sc_time_stamp() << "]" << format::reset;
        }
    });

    m_root_loggers.reconfigure();
}

ConfigManager::~ConfigManager()
{
    delete m_ui;
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

bool ConfigManager::compute_platform(const string &name,
                                     const PlatformDescription &p, PlatformDescription &out)
{
    out = p;

    if (out["inherit"].is_scalar()) {
        const string parent_name = out["inherit"].as<string>();

        LOG(APP, TRC) << "Platform " << name << " inherits from `" << parent_name << "`\n";

        if (platform_exists(parent_name)) {
            bool is_generic = out.exists("generic") && out["generic"].as<bool>();
            PlatformDescription parent = get_platform(parent_name);

            out = out.merge(parent);
            if (!is_generic) {
                out.remove("generic");
            }
        } else {
            LOG(APP, TRC) << "Platform " << name
                << " inherits from unknown platform `" << parent_name << "`\n";
            return false;
        }
    }

    return true;
}

void ConfigManager::recompute_platforms()
{
    m_platforms.clear();
    list< pair< string, PlatformDescription> > todo;

    LOG(APP, TRC) << "Recompute Platforms\n";

    if (!m_root_descr["platforms"].is_map()) {
        LOG(APP, TRC) << "No platform found.\n";
        return;
    }

    for (auto &p : m_root_descr["platforms"]) {
        LOG(APP, TRC) << "Found platform " << p.first << "\n";
        todo.push_back(p);
    }

    bool changes = true;

    while (!todo.empty()) {
        if (!changes) {
            LOG(APP, TRC) << "Some platforms are left un-computed\n";
            break;
        }

        changes = false;

        for (auto it = todo.begin(); it != todo.end();) {
            PlatformDescription out;
            if (compute_platform(it->first, it->second, out)) {
                /* Computing ok */
                m_platforms[it->first] = out;
                it = todo.erase(it);
                changes = true;
            } else {
                it++;
            }
        }
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

    m_root_loggers.reconfigure();
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
        LOG(APP, TRC) << "Trying to deduce selected platform from basename\n";
        const string prefix(RABBITS_PLATFORM_SYMLINK_PREFIX);

        if (basename.find(prefix) != 0) {
            LOG(APP, TRC) << "basename seems invalid. Giving up.\n";
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

void ConfigManager::create_ui(UiChooser::Hint hint)
{
    if (m_ui != nullptr) {
        LOG(APP, DBG) << "Ui already created. Skipping.\n";
        return;
    }

    m_ui = UiChooser::create_ui(hint, *this);
}

Ui & ConfigManager::get_ui()
{
    if (m_ui == nullptr) {
        create_ui();
    }

    return *m_ui;
}
