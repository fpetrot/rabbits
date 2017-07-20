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

typedef Logger & (*format_fn)(Logger &);

static const format_fn LVL_COLOR[] {
    format::red,
    format::cyan,
    format::white,
};

#define LVL_COLOR_SZ    (sizeof(LVL_COLOR) / sizeof(LVL_COLOR[0]))

static inline format_fn get_lvl_color(int lvl)
{
    if (lvl >= int(LVL_COLOR_SZ)) {
        return LVL_COLOR[LVL_COLOR_SZ-1];
    } else {
        return LVL_COLOR[lvl];
    }
}

static void _dump_systemc_hierarchy(const sc_core::sc_object &top_level,
                                    TextFormatter &f, int lvl)
{
    const vector<sc_core::sc_object*> & children = top_level.get_child_objects();
    const format_fn color = get_lvl_color(lvl);

    f << color << top_level.basename()
      << format::reset << ": "
      << format::reset << top_level.kind()
      << format::reset << "\n";

    f.tree_push(color);

    for (auto &c : children) {
        const bool last = (&c == &(children[children.size()-1]));

        if (last) {
            f.tree_set_last_child();
        }

        _dump_systemc_hierarchy(*c, f, lvl+1);
    }

    f.tree_pop();
}


void dump_systemc_hierarchy(PlatformBuilder &p, LogLevel::value lvl)
{
    Logger &l = get_app_logger();
    vector<bool> lvls;

    bool banner_status = l.enable_banner(false);
    l << format::white_b << "SystemC hierarchy:\n\n" << format::reset;

    TextFormatter f(l, lvl);

    f.enable_tree();

    _dump_systemc_hierarchy(p, f, 0);

    l << "\n";
    l.enable_banner(banner_status);
}


