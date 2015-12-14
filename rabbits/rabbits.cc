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

#include <rabbits/ui/ui.h>

#include <rabbits/dynloader/dynloader.h>

#include <boost/filesystem.hpp>

#include <cstdlib>

using boost::filesystem::path;
using boost::filesystem::is_regular_file;
using std::string;

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

extern "C" {
int sc_main(int argc, char *argv[])
{
    DynamicLoader &dyn_loader = DynamicLoader::get();
    PlatformDescription p;
    string yml_path;
    char *env_dynlib_paths;

    env_dynlib_paths = std::getenv("RABBITS_DYNLIB_PATH");
    if (env_dynlib_paths != NULL) {
        dyn_loader.add_semicol_sep_search_paths(env_dynlib_paths);
    }

    p.parse_cmdline(argc, argv);

    yml_path = get_yml_config(p, argv[0]);
    if (yml_path != "") {
        PlatformDescription p_yml;
        DBG_STREAM("Loading config file " << yml_path << "\n");
        p_yml.load_file_yaml(yml_path);
        p = p.merge(p_yml);
    }

    dyn_loader.search_and_load_rabbits_dynlibs();

    PlatformBuilder builder("platform", p);

    Logger::get().set_log_level(Logger::INFO);
    simu_manager().start();

    return 0;
}
}
