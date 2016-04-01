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

#include <iostream>
#include <set>

#include "usage.h"

using std::string;
using std::vector;
using std::set;
using std::cout;
using std::ostream;

static string operator*(const string &s, int i)
{
    string ret;

    while(i--) {
        ret += s;
    }

    return ret;
}

const string INDENT = "   ";
static int indent_step = 0;
static ostream & cout_indent(int istep = indent_step)
{
    return cout << (INDENT*istep);
}

static void describe_comp_params(const ComponentParameters &p)
{
    ComponentParameters::const_iterator it;

    for (it = p.begin(); it != p.end(); it++) {
        cout_indent()
            << "- " << it->first
            << ": " << it->second->get_description() << "\n";
    }
}

static void describe_component(ComponentFactory &c)
{
    const ComponentParameters & p = c.get_params();

    cout_indent() << "type: " << c.type() << "\n";
    cout_indent() << "description: " << c.description() << "\n";

    if (p.empty()) {
        return;
    }

    cout_indent() << "parameters:\n";
    indent_step++;
    describe_comp_params(p);
    indent_step--;
}

void enum_components()
{
    ComponentManager &cm = ComponentManager::get();
    ComponentManager::iterator it;

    cout_indent() << "\nAvailable components:\n\n";

    for (it = cm.begin(); it != cm.end(); it++) {
        cout_indent() << "* " << it->first << "\n";
        indent_step++;
        describe_component(*(it->second));
        indent_step--;
        cout_indent() << "\n";
    }
}

static void print_option(const string &opt, const string& help)
{
    cout_indent(1) << "-" << opt << "\r\t\t\t\t\t" << help << "\n";
}

static void print_option_with_val(const string &opt, const string& help, const string &value)
{
    cout_indent(1) << "-" << opt << "\r\t\t\t\t\t" << help << " [" << value << "]\n";
}

static void print_cmdline_usage(CmdlineInfo &cmdline)
{
    CmdlineInfo::iterator it;

    for (it = cmdline.begin(); it != cmdline.end(); it++) {
        print_option(it->first, it->second.help);
    }
}

static void print_comp_usage(const string &comp, const ComponentParameters &p)
{
    ComponentParameters::const_iterator it;

    for (it = p.begin(); it != p.end(); it++) {
        print_option_with_val("components." + comp + "." + it->first,
                              it->second->get_description(), it->second->to_str());
    }
}

void print_usage(const char* arg0, CmdlineInfo &cmdline, PlatformBuilder &p)
{
    PlatformBuilder::const_comp_iterator it;

    cout << "Usage: " << arg0 << " [...]\n";

    cout << "Arguments:\n";
    print_cmdline_usage(cmdline);

    if (p.is_empty()) {
        return;
    }

    cout << "\nPlatform arguments:\n";
    for (it = p.comp_begin(); it != p.comp_end(); it++) {
        print_comp_usage(it->first, it->second->get_params());
    }
}

void print_version(ostream &s)
{
    s << RABBITS_APP_NAME
        << " version " << RABBITS_VERSION
        << " api version " << RABBITS_API_VERSION << "\n";
}
