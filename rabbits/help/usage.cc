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


#include <rabbits/platform/description.h>
#include <rabbits/platform/builder.h>
#include <rabbits/platform/parser.h>

#include <rabbits/component/factory.h>
#include <rabbits/plugin/plugin.h>

#include <rabbits/ui/ui.h>

#include "help.h"

#include "formatter.h"
#include "usage.h"

using std::string;
using std::vector;

static void add_aliases(ConfigManager &conf, UsageFormatter &usage, bool advanced)
{
    const ConfigManager::ParamAliases &aliases = conf.get_param_aliases();
    usage.add_section("Shortcuts");

    for (const auto alias : aliases) {
        if (advanced || !alias.second->is_advanced()) {
            usage.add_alias(alias.first, *alias.second);
        }
    }
}

static void add_parameters(const Parameters &params, UsageFormatter &usage,
                           bool advanced)
{
    for (const auto & param : params) {
        if (advanced || !param.second->is_advanced()) {
            usage.add_param(*param.second);
        }
    }
}

static void add_global_parameters(ConfigManager &conf, UsageFormatter &usage,
                                  bool advanced)
{
    const Parameters & globals = conf.get_global_params();
    usage.add_section("Rabbits global parameters");

    add_parameters(globals, usage, advanced);
}

static void add_platform_parameters(PlatformBuilder &p, UsageFormatter &usage,
                                    bool advanced)
{
    if (p.is_empty()) {
        return;
    }

    usage.add_section("Platform parameters");
    for (auto & comp : p.get_components()) {
        add_parameters(comp.second->get_params(), usage, advanced);
    }

    for (auto & plug : p.get_plugins()) {
        add_parameters(plug.second->get_params(), usage, advanced);
    }

    for (auto & be : p.get_backends()) {
        add_parameters(be.second->get_params(), usage, advanced);
    }

}

void print_usage(const char* arg0, ConfigManager &conf, PlatformBuilder &p)
{
    UsageFormatter usage;

    bool banner_status = get_app_logger().enable_banner(false);
    bool advanced = conf.get_global_params()["show-advanced-params"].as<bool>();

    LOG(APP, INF) << format::white_b << "Usage: " << arg0 << " [...]\n\n";

    if (advanced) {
        LOG(APP, INF) << format::yellow << "Displaying advanced parameters\n" << format::reset;
    }

    add_aliases(conf, usage, advanced);
    add_global_parameters(conf, usage, advanced);
    add_platform_parameters(p, usage, advanced);

    usage.dump(LogLevel::INFO);

    get_app_logger().enable_banner(banner_status);
}

void print_version(LogLevel::value lvl)
{
    Logger &l = get_app_logger();
    bool banner_status = l.enable_banner(false);

    l.next_trace(lvl);

    l << RABBITS_APP_NAME
        << " version " << RABBITS_VERSION
        << " api version " << RABBITS_API_VERSION << "\n";

    l.enable_banner(banner_status);
}
