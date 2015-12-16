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

#include <boost/filesystem.hpp>

#include <cstdlib>
#include <set>
#include <iostream>

using boost::filesystem::path;
using boost::filesystem::is_regular_file;
using std::string;
using std::vector;
using std::set;
using std::cout;
using std::ostream;

struct CmdlineInfo {
    bool print_usage;
    bool enum_components;

    CmdlineInfo() 
        : print_usage(false)
        , enum_components(false)
    {}
};

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

static void enum_components()
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

static void print_hardcoded_usage()
{
    print_option("help", "Print this message");
    print_option("list-components", "List available components with their description");
}

static void print_comp_usage(const string &comp, const ComponentParameters &p)
{
    ComponentParameters::const_iterator it;

    for (it = p.begin(); it != p.end(); it++) {
        print_option("components." + comp + "." + it->first, it->second->get_description());
    }
}

static void print_usage(const char* arg0, PlatformBuilder &p)
{
    PlatformBuilder::const_comp_iterator it;

    cout << "Usage: " << arg0 << " [...]\n";

    cout << "Arguments:\n";
    print_hardcoded_usage();

    cout << "\nPlatform arguments:\n";
    for (it = p.comp_begin(); it != p.comp_end(); it++) {
        print_comp_usage(it->first, it->second->get_params());
    }
}

static string get_yml_config(PlatformDescription &p, const char *arg0)
{
    if (p["config"].is_scalar()) {
        return p["config"].as<string>();
    }

    DBG_STREAM("No -config option provided, trying with basename\n");

    const string basename = path(arg0).filename().string();

    if (basename != RABBITS_APP_NAME) {
        const string search_path(RABBITS_DESCR_SEARCH_PATH);
        const string final_path = search_path + "/" + basename + ".yml";

        if (is_regular_file(final_path)) {
            return final_path;
        } else {
            DBG_STREAM(final_path << " not found.\n");
        }
    }

    return "";
}

static void parse_arg(string arg, string val, CmdlineInfo &cmdline)
{
    if (arg == "help") {
        cmdline.print_usage = true;
    } else if (arg == "list-components") {
        cmdline.enum_components = true;
    }
}

static void parse_cmdline(int argc, char *argv[],
                          PlatformDescription &p, CmdlineInfo &cmdline)
{
    set<string> unaries;

    unaries.insert("help");
    unaries.insert("list-components");

    p.parse_cmdline(argc, argv, unaries);

    if (p.is_map()) {
        PlatformDescription::iterator it;

        for (it = p.begin(); it != p.end(); it++) {
            if (it->second.is_scalar()) {
                parse_arg(it->first, it->second.as<string>(), cmdline);
            }
        }
    }
}

extern "C" {
int sc_main(int argc, char *argv[])
{
    CmdlineInfo cmdline;
    DynamicLoader &dyn_loader = DynamicLoader::get();
    PlatformDescription p;
    string yml_path;
    char *env_dynlib_paths;

    env_dynlib_paths = std::getenv("RABBITS_DYNLIB_PATH");
    if (env_dynlib_paths != NULL) {
        dyn_loader.add_semicol_sep_search_paths(env_dynlib_paths);
    }

    parse_cmdline(argc, argv, p, cmdline);

    yml_path = get_yml_config(p, argv[0]);
    if (yml_path != "") {
        PlatformDescription p_yml;
        DBG_STREAM("Loading config file " << yml_path << "\n");
        p_yml.load_file_yaml(yml_path);
        p = p.merge(p_yml);
    }

    dyn_loader.search_and_load_rabbits_dynlibs();

    if (cmdline.enum_components) {
        enum_components();
        return 0;
    }

    PlatformBuilder builder("platform", p);

    if (cmdline.print_usage) {
        print_usage(argv[0], builder);
        return 0;
    }

    Logger::get().set_log_level(Logger::INFO);
    simu_manager().start();

    return 0;
}
}
