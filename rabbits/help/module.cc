/*
 *  This file is part of Rabbits
 *  Copyright (C) 2017 Luc MICHEL - Antfield SAS
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

#include "help.h"
#include "formatter.h"

static void describe_comp_params(const Parameters &p, TextFormatter &f)
{
    Parameters::const_iterator it;

    for (it = p.begin(); it != p.end(); it++) {
        f.set_start_col(4);
        f << format::green << it->first << format::reset;

        f.set_start_col(5 + it->first.size());
        f << strip_last_nl(it->second->get_description()) << "\n";
    }
}

static void describe_module(ModuleFactoryBase &c, TextFormatter &f)
{
    const Parameters & p = c.get_params();
    ModuleFactoryBase::ExtraValues values;

    f.set_max_len(80);

    c.get_extra_values(values);
    for (auto &v : values) {
        f.set_start_col(2);
        print_value(f, v.first, v.second);
    }

    f.set_start_col(2);
    print_value(f, "description", c.get_description());

    if (p.empty()) {
        return;
    }

    f.set_start_col(2);
    f << format::cyan << "parameters:\n" << format::reset;
    describe_comp_params(p, f);
}

void enum_modules(ConfigManager &config, const Namespace &ns, LogLevel::value lvl)
{
    ModuleManagerBase &man = config.get_manager_by_namespace(ns);

    Logger &l = get_app_logger();

    bool banner_status = l.enable_banner(false);

    TextFormatter f(l, lvl);

    f << format::white_b << "Available " << ns.get_name() << ":\n\n"
      << format::reset;

    for (auto &m : man) {
        f.set_start_col(0);
        f << format::cyan_b << "* "
          << format::white_b << m.first
          << format::reset << "\n";

        describe_module(*(m.second), f);

        f << "\n";
    }

    l << "\n";
    l.enable_banner(banner_status);
}


