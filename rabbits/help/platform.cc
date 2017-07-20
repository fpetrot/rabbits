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

using std::vector;

static void enum_platform(PlatformParser &parser, TextFormatter &f)
{
    f.set_start_col(2);
    print_value(f, "description", parser.get_root().get_description());

    if (parser.get_root().has_parent()) {
        print_value(f, "inherits from", parser.get_root().get_parent_name());
    }

    if (parser.get_root().is_generic()) {
        f << format::green << "This is a generic platform\n";
    }

    f << "\n";
}

void enum_platforms(ConfigManager &config, LogLevel::value lvl, bool show_generics)
{
    Logger &l = get_app_logger();

    bool banner_status = l.enable_banner(false);

    TextFormatter f(l, lvl);

    f << format::white_b << "Available platforms:\n\n"
      << format::reset;

    for (auto p : config.get_platforms()) {
        try {
            PlatformParser parser(p.first, p.second, config);

            if (show_generics || !parser.get_root().is_generic()) {
                f.set_start_col(0);
                f << format::cyan_b << "* "
                    << format::white_b << p.first
                    << format::reset << "\n";

                enum_platform(parser, f);
            }
        } catch (PlatformParseException e) {
            f.set_start_col(0);
            f << format::cyan_b << "* "
                << format::white_b << p.first
                << format::reset << "\n";

            f.set_start_col(2);
            f << format::red_b << "Parsing error: "
              << format::red << e.what()
              << format::reset << "\n\n";
        }
    }

    l << "\n";
    l.enable_banner(banner_status);
}


static void describe_port(Port &p, TextFormatter &f)
{
    const vector<PortConnectionInfo> & con = p.get_connections_info();

    f << "\n";
    f.tree_push(format::purple);

    switch (con.size()) {
    case 0:
        f.tree_set_last_child();
        f << format::red_b << "not connected"
          << format::reset << "\n";
        break;
    default:
        for (auto &c: con) {
            if (&c == &con.back()) {
                f.tree_set_last_child();
            }

            ComponentBase &c_peer = c.peer->get_parent()->get_component();

            const Namespace &n_peer = c_peer.get_namespace();
            const Namespace &n_local = p.get_parent()->get_component().get_namespace();

            f << format::green << "connected to ";

            if (n_peer.get_id() != n_local.get_id()) {
                f << format::cyan_b << n_peer.get_name() << ":";
            }

            f << format::cyan << c_peer.basename()
              << format::reset << "."
              << format::purple << c.peer->name()
              << format::reset << "\n";

            f.tree_push(nullptr);

            f << "using "
              << format::blue << c.cs->get_typeid()
              << format::reset << " connection strategy\n";

            for (const auto &i: c.extra_info) {
                f << format::blue << i.first << ": "
                  << format::reset << i.second << "\n";
            }

            f.tree_pop();
        }
        break;
    }

    f.tree_pop();
}

static void describe_component(ComponentBase &c, TextFormatter &f)
{
    ModuleFactoryBase *fac = c.get_factory();

    f << format::cyan << c.basename()
      << format::reset << ": " << fac->get_type() << "\n";


    f.tree_push(format::cyan);
    for (auto it = c.port_begin(); it != c.port_end(); it++) {
        auto next_it = it;
        next_it++;

        if (next_it == c.port_end()) {
            f.tree_set_last_child();
        }

        f << format::purple << it->first
          << format::reset << ": " << it->second->get_typeid();
        describe_port(*it->second, f);
    }
    f.tree_pop();
}

void describe_platform(PlatformBuilder &platform)
{
    Logger &l = get_app_logger();

    bool banner_status = l.enable_banner(false);
    TextFormatter f(l, LogLevel::INFO);

    f.enable_tree();

    f << format::white_b << "Platform " << platform.name() << "\n"
      << format::reset;

    f << format::white_b << "components" << "\n"
      << format::reset;

    f.tree_push(format::white_b);
    for (auto it : platform.get_components()) {
        if (it.second == platform.get_components().rbegin()->second) {
            f.tree_set_last_child();
        }

        describe_component(*it.second, f);
    }
    f.tree_pop();

    f << "\n" << format::white_b << "backends" << "\n"
      << format::reset;

    f.tree_push(format::white_b);
    for (auto it : platform.get_backends()) {
        if (it.second == platform.get_backends().rbegin()->second) {
            f.tree_set_last_child();
        }

        describe_component(*it.second, f);
    }
    f.tree_pop();

    l.enable_banner(banner_status);
}
