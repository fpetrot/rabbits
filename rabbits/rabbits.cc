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

#include <cstdlib>
#include <iostream>

#include "usage.h"
#include "cmdline.h"

using std::string;
using std::vector;

static void dump_systemc_hierarchy(const sc_core::sc_object &top_level, int indent = 0)
{
    const vector<sc_core::sc_object*> & children = top_level.get_child_objects();
    vector<sc_core::sc_object*>::const_iterator it;

    //print_indent(indent);
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
    get_app_logger().set_log_level(LogLevel::INFO);
    get_sim_logger().set_log_level(LogLevel::INFO);

    ConfigManager config;

    declare_global_params(config);
    declare_aliases(config);

    config.add_cmdline(argc, argv);

    ComponentParameters &globals = config.get_global_params();

    if (globals["debug"].as<bool>()) {
        get_app_logger().set_log_level(LogLevel::DEBUG);
        get_sim_logger().set_log_level(LogLevel::DEBUG);
        /* XXX */
        print_version(std::cerr);
    }

    if (globals["show-version"].as<bool>()) {
        /* XXX */
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
        if (globals["show-help"].as<bool>()) {
            PlatformBuilder empty("", PlatformDescription::INVALID_DESCRIPTION);
            print_usage(argv[0], config, empty);
            return 0;
        }
        LOG(APP, ERR) << "No selected platform. Please select a platform with -platform. Try -help.\n";
        return 1;
    }

    if (!config.platform_exists(pname)) {
        LOG(APP, ERR) << "Platform " << pname << " not found. Try -help.\n";
        return 1;
    }

    LOG(APP, DBG) << "Selected platform is " << pname << "\n";

    PlatformDescription platform = config.apply_platform(pname);
    PlatformBuilder builder(pname.c_str(), platform);

    if (globals["show-help"].as<bool>()) {
        print_usage(argv[0], config, builder);
        return 0;
    }

    simu_manager().start();

    return 0;
}
}
