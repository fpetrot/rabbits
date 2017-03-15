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

    config.add_global_param("list-all-platforms",
                            Parameter<bool>("List all available platforms, "
                                            "including the generic ones",
                                            false,
                                            true));

    config.add_global_param("show-systemc-hierarchy",
                            Parameter<bool>("Display the SystemC hierarchy "
                                            "at the end of elaboration and exit",
                                            false,
                                            true));

    config.add_global_param("show-version",
                            Parameter<bool>("Display version information and exit",
                                            false));

    config.add_global_param("disable-gui",
                            Parameter<bool>("Completly disable the GUI", false));
}

static void declare_aliases(ConfigManager &config)
{
    Parameters &p = config.get_global_params();

    config.add_param_alias("help",               p["show-help"]);
    config.add_param_alias("help-advanced",      p["show-advanced-params"]);
    config.add_param_alias("list-components",    p["list-components"]);
    config.add_param_alias("list-backends",      p["list-backends"]);
    config.add_param_alias("list-plugins",       p["list-plugins"]);
    config.add_param_alias("list-platforms",     p["list-platforms"]);
    config.add_param_alias("list-all-platforms", p["list-all-platforms"]);
    config.add_param_alias("systemc-hierarchy",  p["show-systemc-hierarchy"]);
    config.add_param_alias("debug",              p["debug"]);
    config.add_param_alias("version",            p["show-version"]);
    config.add_param_alias("platform",           p["selected-platform"]);
    config.add_param_alias("nographic",          p["disable-gui"]);
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

enum eRunMode {
    RUN_NORMAL,
    RUN_HELP,
    RUN_VERSION,
    RUN_LIST_PLATFORMS,
    RUN_LIST_ALL_PLATFORMS,
    RUN_LIST_COMPONENTS,
    RUN_LIST_BACKENDS,
    RUN_LIST_PLUGINS,
    RUN_SYSC_HIERARCHY
};

static eRunMode get_run_mode(Parameters &globals)
{
    if (globals["show-version"].as<bool>()) {
        return RUN_VERSION;
    }

    if (globals["list-components"].as<bool>()) {
        return RUN_LIST_COMPONENTS;
    }

    if (globals["list-backends"].as<bool>()) {
        return RUN_LIST_BACKENDS;
    }

    if (globals["list-plugins"].as<bool>()) {
        return RUN_LIST_PLUGINS;
    }

    if (globals["list-platforms"].as<bool>()) {
        return RUN_LIST_PLATFORMS;
    }

    if (globals["list-all-platforms"].as<bool>()) {
        return RUN_LIST_ALL_PLATFORMS;
    }

    if (globals["show-help"].as<bool>()) {
        return RUN_HELP;
    }

    if (globals["show-advanced-params"].as<bool>()) {
        return RUN_HELP;
    }

    if (globals["show-systemc-hierarchy"].as<bool>()) {
        return RUN_SYSC_HIERARCHY;
    }

    return RUN_NORMAL;
}

static UiChooser::Hint get_ui_hint(eRunMode run_mode, Parameters &globals)
{
    if (run_mode != RUN_NORMAL) {
        /* Non-normal modes require headless */
        return UiChooser::HEADLESS;
    }

    if (globals["disable-gui"].as<bool>()) {
        /* User asked for headless */
        return UiChooser::HEADLESS;
    }

    return UiChooser::AUTO;
}

static void load_modules(ConfigManager &config)
{
    StaticLoader::load(config);

    DynamicLoader &dyn_loader = config.get_dynloader();
    char * env_dynlib_paths = std::getenv("RABBITS_DYNLIB_PATH");
    if (env_dynlib_paths != NULL) {
        dyn_loader.add_colon_sep_search_paths(env_dynlib_paths);
    }

    dyn_loader.search_and_load_rabbits_dynlibs();
}

static bool list_modules(ConfigManager &config, eRunMode mode)
{
    switch (mode) {
    case RUN_LIST_COMPONENTS:
        enum_modules(config, Namespace::get(Namespace::COMPONENT), LogLevel::INFO);
        break;
    case RUN_LIST_BACKENDS:
        enum_modules(config, Namespace::get(Namespace::BACKEND), LogLevel::INFO);
        break;
    case RUN_LIST_PLUGINS:
        enum_modules(config, Namespace::get(Namespace::PLUGIN), LogLevel::INFO);
        break;
    case RUN_LIST_PLATFORMS:
        enum_platforms(config, LogLevel::INFO, false);
        break;
    case RUN_LIST_ALL_PLATFORMS:
        enum_platforms(config, LogLevel::INFO, true);
        break;
    default:
        return false;
    }

    return true;
}

int sc_main(int argc, char *argv[])
{
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

    const eRunMode run_mode = get_run_mode(globals);
    const UiChooser::Hint ui_hint = get_ui_hint(run_mode, globals);

    config.create_ui(ui_hint);

    if (globals["debug"].as<bool>()) {
        print_version(LogLevel::DEBUG);
    }

    if (run_mode == RUN_VERSION) {
        print_version(LogLevel::INFO);
        return 0;
    }

    load_modules(config);

    if (list_modules(config, run_mode)) {
        return 0;
    }

    std::string pname = globals["selected-platform"].as<string>();

    if (pname.empty()) {
        if (run_mode == RUN_HELP) {
            PlatformBuilder empty("", config);
            print_usage(argv[0], config, empty);
            return 0;
        }
        LOG(APP, ERR) << "No selected platform. "
            "Please select a platform with -platform. Try -help.\n";
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

        if (run_mode == RUN_HELP) {
            print_usage(argv[0], config, builder);
            return 0;
        }

        check_unused_params(platform);

        if (run_mode == RUN_SYSC_HIERARCHY) {
            dump_systemc_hierarchy(builder, LogLevel::INFO);
            return 0;
        }

        SimuManager(config).start();

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

    return 0;
}
