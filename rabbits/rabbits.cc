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

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <memory>

#include "usage.h"
#include "cmdline.h"

using std::string;
using std::vector;
using std::map;
using std::fstream;
using std::unique_ptr;

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

    config.add_global_param("debug",
                            Parameter<bool>("Set log level to `debug' "
                                            "(equivalent to `-log-level debug')",
                                            false));

    config.add_global_param("show-version",
                            Parameter<bool>("Display version information and exit",
                                            false));

    /* Logger stuff */
    config.add_global_param("log-target",
                            Parameter<string>("Specify the log target (valid options "
                                              "are `stdout', `stderr' and `file')",
                                              "stderr"));
    config.add_global_param("log-file",
                            Parameter<string>("Specify the log file",
                                              "rabbits.log"));
    config.add_global_param("log-level",
                            Parameter<string>("Specify the log level (valid options "
                                              "are `trace', `debug', `info', `warning', `error')",
                                              "info"));
    config.add_global_param("log-sim-target",
                            Parameter<string>("Specify simulation related log target "
                                              "(valid options are `stdout', `stderr' and `file')",
                                              "stderr"));
    config.add_global_param("log-sim-file",
                            Parameter<string>("Specify simulation related log file",
                                              "rabbits.log"));
    config.add_global_param("log-sim-level",
                            Parameter<string>("Specify simulation related log level "
                                              "(valid options are `trace', `debug', `info', `warning', `error')",
                                              "info"));
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

enum LogTarget {
    LT_STDOUT, LT_STDERR, LT_FILE
};

typedef unique_ptr<fstream> LogFile;
typedef map<string, LogFile> LogFiles;

static LogTarget get_log_target(const string target_s)
{
    if (target_s == "stdout") {
        return LT_STDOUT;
    } else if (target_s == "stderr") {
        return LT_STDERR;
    } else if (target_s == "file") {
        return LT_FILE;
    }

    LOG(APP, ERR) << "Ignoring invalid log target " << target_s << "\n";
    return LT_STDERR;
}

static LogLevel::value get_log_level(const string level_s)
{
    if (level_s == "debug") {
        return LogLevel::DEBUG;
    } else if (level_s == "info") {
        return LogLevel::INFO;
    } else if (level_s == "warning") {
        return LogLevel::WARNING;
    } else if (level_s == "error") {
        return LogLevel::ERROR;
    } else if (level_s == "trace") {
        return LogLevel::TRACE;
    }

    LOG(APP, ERR) << "Ignoring invalid log level " << level_s << "\n";
    return LogLevel::INFO;
}

static fstream* open_file(const string &fn, LogFiles &files)
{
    fstream* ret = nullptr;

    if (files.find(fn) != files.end()) {
        if (!files[fn]) {
            return nullptr;
        }
        return files[fn].get();
    }

    ret = new fstream(fn, fstream::out | fstream::trunc);

    files[fn].reset(ret);

    return ret;
}

static void setup_logger(Logger &l, LogTarget target, LogLevel::value lvl,
                         const string log_file, LogFiles &files)
{
    switch (target) {
    case LT_STDOUT:
        l.set_streams(&std::cout);
        break;

    case LT_FILE:
        {
            fstream* file = open_file(log_file, files);

            if (!*file) {
                LOG(APP, ERR) << "Unable to open log file " << log_file << ". Falling back to stderr\n";
            } else {
                l.set_streams(file);
            }
        }
        break;

    case LT_STDERR:
        /* Default */
        break;
    }

    l.set_log_level(lvl);
}

static inline bool sim_logger_is_custom(Parameters &g)
{
    return (!g["log-sim-target"].is_default())
        || (!g["log-sim-level"].is_default())
        || (!g["log-sim-file"].is_default());
}

static void setup_loggers(ConfigManager &config, LogFiles &files)
{
    Parameters &p = config.get_global_params();
    Logger &app = get_app_logger();
    Logger &sim = get_sim_logger();

    const LogTarget log_target = get_log_target(p["log-target"].as<string>());
    const LogLevel::value log_level = get_log_level(p["log-level"].as<string>());
    const string log_file = p["log-file"].as<string>();

    const LogTarget log_sim_target = get_log_target(p["log-sim-target"].as<string>());
    const LogLevel::value log_sim_level = get_log_level(p["log-sim-level"].as<string>());
    const string log_sim_file = p["log-sim-file"].as<string>();

    setup_logger(app, log_target, log_level, log_file, files);

    if (sim_logger_is_custom(p)) {
        setup_logger(sim, log_sim_target, log_sim_level, log_sim_file, files);
    } else {
        setup_logger(sim, log_target, log_level, log_file, files);
    }

    sim.set_custom_banner([] (Logger &l, const std::string &banner)
    {
            l << format::purple << "[sim]";
        if (sc_core::sc_get_status() == sc_core::SC_ELABORATION) {
            l << format::green << "[elaboration]" << format::reset;
        } else {
            l << format::green << "[" << sc_core::sc_time_stamp() << "]" << format::reset;
        }
    });
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
                     const std::vector<std::string> &names) {
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

    get_app_logger().set_log_level(LogLevel::INFO);
    get_sim_logger().set_log_level(LogLevel::INFO);

    declare_global_params(config);
    declare_aliases(config);

    try {
        config.add_cmdline(argc, argv);
    } catch (PlatformDescription::InvalidCmdLineException e) {
        LOG(APP, ERR) << e.what() << ". Try -help\n";
        return 1;
    }

    Parameters &globals = config.get_global_params();

    LogFiles log_files;

    if (globals["debug"].as<bool>()) {
        globals["log-level"] = string("debug");
        globals["log-sim-level"] = string("debug");
    }

    setup_loggers(config, log_files);

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
