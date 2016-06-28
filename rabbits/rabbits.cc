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

#include <rabbits/logger.h>
#include <rabbits/simu.h>

#include <rabbits/platform/description.h>
#include <rabbits/platform/builder.h>

#include <rabbits/component/factory.h>

#include <rabbits/ui/ui.h>

#include <rabbits/dynloader/dynloader.h>

#include <rabbits/config/manager.h>

#include <boost/filesystem.hpp>

#include <cstdlib>
#include <set>
#include <iostream>

#include "usage.h"
#include "cmdline.h"

using boost::filesystem::path;
using boost::filesystem::is_regular_file;
using std::string;
using std::vector;
using std::set;

static void get_yml_from_basename(const char *arg0, vector<string> & files)
{

    const string basename = path(arg0).filename().string();

    if (basename != RABBITS_APP_NAME) {
        DBG_STREAM("Trying to deduce config from basename\n");
        const string search_path(RABBITS_DESCR_SEARCH_PATH);
        const string prefix(RABBITS_DESCR_SYMLINK_PREFIX);

        if (basename.find(prefix) != 0) {
            DBG_STREAM("basename seems invalid. Giving up.\n");
            return;
        }

        path fp(search_path);
        fp /= path(basename.substr(prefix.size()) + ".yml");

        const string final_path = fp.string();

        if (is_regular_file(final_path)) {
            files.push_back(final_path);
        } else {
            DBG_STREAM(final_path << " not found.\n");
        }
    }
}

static void get_yml_from_config(PlatformDescription &p, vector<string> & files)
{
    if (p.is_scalar()) {
        files.push_back(p.as<string>());
    } else if (p.is_map()) {
        PlatformDescription::iterator it;

        for (it = p.begin(); it != p.end(); it++) {
            get_yml_from_config(it->second, files);
        }
    }
}

static void get_yml_configs(PlatformDescription &p, const char *arg0, vector<string> & files)
{
    get_yml_from_basename(arg0, files);
    get_yml_from_config(p["config"], files);

    DBG_STREAM("Got " << files.size() << " config file(s) to load\n");
}

static void build_description(PlatformDescription& p, const char *arg0)
{
    vector<string> configs;
    vector<string>::iterator it;

    get_yml_configs(p, arg0, configs);

    for (it = configs.begin(); it != configs.end(); it++) {
        PlatformDescription p_yml;

        DBG_STREAM("Loading " << *it << "\n");
        p_yml.load_file_yaml(*it);
        p = p.merge(p_yml);
    }
}

static void map_to_set(const CmdlineInfo &in, set<string> &out)
{
    CmdlineInfo::const_iterator it;

    for (it = in.begin(); it != in.end(); it++) {
        out.insert(it->first);
    }
}

static void parse_cmdline(int argc, char *argv[],
                          PlatformDescription &p, CmdlineInfo &cmdline)
{
    set<string> unaries;

    map_to_set(cmdline, unaries);

    p.parse_cmdline(argc, argv, unaries);

    if (p.is_map()) {
        CmdlineInfo::iterator it;

        for (it = cmdline.begin(); it != cmdline.end(); it++) {
            if (p[it->first].is_scalar()) {
                it->second.value = true;
            }
        }
    }
}

static void build_cmdline(CmdlineInfo &cmdline)
{
    cmdline["help"] = CmdlineEntry("This message");
    cmdline["list-components"] = CmdlineEntry("List available components with their description");
    cmdline["debug"] = CmdlineEntry("Enable debug messages");
    cmdline["version"] = CmdlineEntry("Print the version and exit");
}

static void print_indent(int indent)
{
    for (int i = 0; i < indent; i++) {
        std::cout << "-- ";
    }
}

static void dump_systemc_hierarchy(const sc_core::sc_object &top_level, int indent = 0)
{
    const vector<sc_core::sc_object*> & children = top_level.get_child_objects();
    vector<sc_core::sc_object*>::const_iterator it;

    print_indent(indent);
    std::cout << top_level.basename() << std::endl;

    for (it = children.begin(); it != children.end(); it++) {
        dump_systemc_hierarchy(**it, indent+1);
    }
}

static void declare_global_params(ConfigManager &config)
{
    config.add_global_param("show-help",
                            Parameter<bool>("Display this help text and exit",
                                            false));

    config.add_global_param("list-components",
                            Parameter<bool>("List available components "
                                            "with their description",
                                            false));

    config.add_global_param("debug",
                            Parameter<bool>("Enable debug messages",
                                            false));

    config.add_global_param("show-version",
                            Parameter<bool>("Display version information and exit",
                                            false));
}

static void declare_aliases(ConfigManager &config)
{
    ComponentParameters &p = config.get_global_params();

    config.add_param_alias("help",            p["show-help"]);
    config.add_param_alias("list-components", p["list-components"]);
    config.add_param_alias("debug",           p["debug"]);
    config.add_param_alias("version",         p["show-version"]);
    config.add_param_alias("platform",        p["selected-platform"]);
}

extern "C" {
int sc_main(int argc, char *argv[])
{
    Logger::get().set_log_level(LogLevel::INFO);

    ConfigManager config;

    declare_global_params(config);
    declare_aliases(config);

    config.add_cmdline(argc, argv);

    ComponentParameters &globals = config.get_global_params();

    if (globals["debug"].as<bool>()) {
        Logger::get().set_log_level(LogLevel::DEBUG);
        print_version(Logger::get().log_stream(LogLevel::DEBUG));
    }

    if (globals["show-version"].as<bool>()) {
        print_version(std::cout);
        return 0;
    }

    DynamicLoader &dyn_loader = DynamicLoader::get();
    char * env_dynlib_paths = std::getenv("RABBITS_DYNLIB_PATH");
    if (env_dynlib_paths != NULL) {
        dyn_loader.add_colon_sep_search_paths(env_dynlib_paths);
    }

    dyn_loader.search_and_load_rabbits_dynlibs();

    if (globals["list-components"].as<bool>()) {
        enum_components();
        return 0;
    }

    std::string pname = globals["selected-platform"].as<string>();

    if (pname.empty()) {
        ERR_STREAM("No selected platform. Please select a platform with -platform. Try -help.\n");
        return 1;
    }

    if (!config.platform_exists(pname)) {
        ERR_STREAM("Platform " << pname << " not found. Try -help.\n");
        return 1;
    }

    DBG_STREAM("Selected platform is " << pname << "\n");

    PlatformDescription platform = config.apply_platform(pname);
    PlatformBuilder(pname.c_str(), platform);

    if (globals["show-help"].as<bool>()) {
        //print_usage(argv[0], cmdline, builder);
        return 0;
    }

    simu_manager().start();

    return 0;
}
}
