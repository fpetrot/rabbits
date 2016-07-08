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

    config.add_global_param("color-output",
                            Parameter<bool>("Allow usage of colors when the "
                                            "output is a terminal",
                                            true));

    config.add_global_param("report-non-mapped-access",
                            Parameter<bool>("Report a simulation error when an initiator tries "
                                            "to access a memory address that lead to a "
                                            "non-mapped area on a bus.",
                                            true,
                                            true));
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
                                              "are `debug', `info', `warning', `error')",
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
                                              "(valid options are `debug', `info', `warning', `error')",
                                              "info"));
}

static void declare_aliases(ConfigManager &config)
{
    ComponentParameters &p = config.get_global_params();

    config.add_param_alias("help",              p["show-help"]);
    config.add_param_alias("help-advanced",     p["show-advanced-params"]);
    config.add_param_alias("list-components",   p["list-components"]);
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

static inline bool sim_logger_is_custom(ComponentParameters &g)
{
    return (!g["log-sim-target"].is_default())
        || (!g["log-sim-level"].is_default())
        || (!g["log-sim-file"].is_default());
}

static void setup_loggers(ConfigManager &config, LogFiles &files)
{
    ComponentParameters &p = config.get_global_params();
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

extern "C" {
int sc_main(int argc, char *argv[])
{
    get_app_logger().set_log_level(LogLevel::INFO);
    get_sim_logger().set_log_level(LogLevel::INFO);

    ConfigManager config;

    ConfigManager::set_manager(config);

    declare_global_params(config);
    declare_aliases(config);

    config.add_cmdline(argc, argv);

    ComponentParameters &globals = config.get_global_params();

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

    DynamicLoader &dyn_loader = DynamicLoader::get();
    char * env_dynlib_paths = std::getenv("RABBITS_DYNLIB_PATH");
    if (env_dynlib_paths != NULL) {
        dyn_loader.add_colon_sep_search_paths(env_dynlib_paths);
    }

    dyn_loader.search_and_load_rabbits_dynlibs();

    if (globals["list-components"].as<bool>()) {
        enum_components(LogLevel::INFO);
        return 0;
    }

    std::string pname = globals["selected-platform"].as<string>();

    if (pname.empty()) {
        if (globals["show-help"].as<bool>() || globals["show-advanced-params"].as<bool>()) {
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

    if (globals["show-help"].as<bool>() || globals["show-advanced-params"].as<bool>()) {
        print_usage(argv[0], config, builder);
        return 0;
    }

    if (globals["show-systemc-hierarchy"].as<bool>()) {
        dump_systemc_hierarchy(builder, LogLevel::INFO);
        return 0;
    }

    simu_manager().start();

    return 0;
}
}
