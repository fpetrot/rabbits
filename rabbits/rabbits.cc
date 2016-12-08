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
#include <rabbits/platform/parser.h>

#include <rabbits/component/factory.h>

#include <rabbits/ui/ui.h>

#include <rabbits/dynloader/dynloader.h>

#include <rabbits/config/manager.h>
#include <rabbits/config/static_loader.h>

#include "usage.h"
#include "cmdline.h"

using std::string;
using std::vector;

static void declare_global_params(ConfigManager &config)
{
    config.add_global_param("show-help",
                            Parameter<bool>("Display this help text and exit",
                                            false));

    config.add_global_param("show-advanced-params",
                            Parameter<bool>("Display the advanced parameters",
                                            false));

    config.add_global_param("list-components",
                            Parameter<bool>("List available components "
                                            "with their description",
                                            false));

    config.add_global_param("list-backends",
                            Parameter<bool>("List available backends "
                                            "with their description",
                                            false));

    config.add_global_param("list-plugins",
                            Parameter<bool>("List available plugins "
                                            "with their description",
                                            false));

    config.add_global_param("list-platforms",
                            Parameter<bool>("List available platforms "
                                            "with their description",
                                            false));

    config.add_global_param("show-systemc-hierarchy",
                            Parameter<bool>("Display the SystemC hierarchy "
                                            "at the end of elaboration and exit",
                                            false,
                                            true));

    config.add_global_param("show-version",
                            Parameter<bool>("Display version information and exit",
                                            false));

}

static void declare_aliases(ConfigManager &config)
{
    Parameters &p = config.get_global_params();

    config.add_param_alias("help",              p["show-help"]);
    config.add_param_alias("help-advanced",     p["show-advanced-params"]);
    config.add_param_alias("list-components",   p["list-components"]);
    config.add_param_alias("list-backends",     p["list-backends"]);
    config.add_param_alias("list-plugins",      p["list-plugins"]);
    config.add_param_alias("list-platforms",    p["list-platforms"]);
    config.add_param_alias("systemc-hierarchy", p["show-systemc-hierarchy"]);
    config.add_param_alias("debug",             p["debug"]);
    config.add_param_alias("version",           p["show-version"]);
    config.add_param_alias("platform",          p["selected-platform"]);
}

class WarnUnusedParams : public PlatformDescription::NodeVisitor {
protected:
    void from_cmdline(const PlatformDescription::Node::Origin &o,
                        const vector<string> &names, string &s)
    {
        string prefix = "Unknown command line parameter `-";
        for (auto &n: names) {
            s += prefix + n;
            prefix = ".";
        }
        s += "`";
    }

    void from_file(const PlatformDescription &d,
                     const vector<string> &names, string &s)
    {
        s = "Unknown parameter `" + names.back() + "` at " + d.origin();
    }

    bool names_to_str(PlatformDescription &d,
                        const vector<string> &names, string &s)
    {
        const PlatformDescription::Node::Origin &o = d.get_origin();

        switch(o.origin) {
        case PlatformDescription::Node::Origin::CMDLINE:
            from_cmdline(o, names, s);
            return true;
        case PlatformDescription::Node::Origin::FILE:
            from_file(d, names, s);
            return true;
        default:
            return false;
        }
    }

public:
    void operator() (PlatformDescription &n,
                     const std::vector<std::string> &names)
    {
        string s;

        if (names_to_str(n, names, s)) {
            LOG(APP, WRN) << s << "\n";
        }
    }
};

void check_unused_params(PlatformDescription &d)
{
    PlatformDescription dc = d.clone();
    dc.remove("platforms");

    WarnUnusedParams warn_unused;
    dc.visit_non_converted(warn_unused);
}

extern "C" {
int sc_main(int argc, char *argv[])
{
    ui::create_ui();

    ConfigManager config;
    ConfigManager::set_config_manager(config);

    declare_global_params(config);
    declare_aliases(config);

    try {
        config.add_cmdline(argc, argv);
    } catch (PlatformDescription::InvalidCmdLineException e) {
        LOG(APP, ERR) << e.what() << ". Try -help\n";
        return 1;
    }

    Parameters &globals = config.get_global_params();

    if (globals["debug"].as<bool>()) {
        print_version(LogLevel::DEBUG);
    }

    if (globals["show-version"].as<bool>()) {
        print_version(LogLevel::INFO);
        return 0;
    }

    StaticLoader::load(config);

    DynamicLoader &dyn_loader = config.get_dynloader();
    char * env_dynlib_paths = std::getenv("RABBITS_DYNLIB_PATH");
    if (env_dynlib_paths != NULL) {
        dyn_loader.add_colon_sep_search_paths(env_dynlib_paths);
    }

    dyn_loader.search_and_load_rabbits_dynlibs();

    if (globals["list-components"].as<bool>()) {
        enum_modules(config, Namespace::get(Namespace::COMPONENT), LogLevel::INFO);
        return 0;
    }

    if (globals["list-backends"].as<bool>()) {
        enum_modules(config, Namespace::get(Namespace::BACKEND), LogLevel::INFO);
        return 0;
    }

    if (globals["list-plugins"].as<bool>()) {
        enum_modules(config, Namespace::get(Namespace::PLUGIN), LogLevel::INFO);
        return 0;
    }

    if (globals["list-platforms"].as<bool>()) {
        enum_platforms(config, LogLevel::INFO);
        return 0;
    }

    std::string pname = globals["selected-platform"].as<string>();

    if (pname.empty()) {
        if (globals["show-help"].as<bool>() || globals["show-advanced-params"].as<bool>()) {
            PlatformBuilder empty("", config);
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

    try {
        PlatformBuilder builder(pname.c_str(), platform, config);

        if (globals["show-help"].as<bool>() || globals["show-advanced-params"].as<bool>()) {
            print_usage(argv[0], config, builder);
            return 0;
        }

        check_unused_params(platform);

        if (globals["show-systemc-hierarchy"].as<bool>()) {
            dump_systemc_hierarchy(builder, LogLevel::INFO);
            return 0;
        }

        simu_manager().start();

    } catch (PlatformParseException e) {
        get_app_logger().enable_banner(false);
        LOG(APP, ERR) << format::red_b << "Error while parsing platform " << pname << ": "
                      << format::reset << e.what() << "\n";
        return 1;
    } catch (RabbitsException e) {
        get_app_logger().enable_banner(false);
        LOG(APP, ERR) << "Fatal Rabbits exception: " << e.what() << "\n"
            << "\nbacktrace:\n" << e.get_backtrace();
        return 1;
    }

    ui::dispose_ui();

    return 0;
}
}
