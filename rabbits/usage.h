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

#ifndef _RABBITS_RABBITS_USAGE_H
#define _RABBITS_RABBITS_USAGE_H

#include <ostream>
#include <rabbits/config/manager.h>
#include <rabbits/platform/builder.h>
#include <rabbits/logger.h>

void print_version(LogLevel::value);
void print_usage(const char* arg0, ConfigManager &config, PlatformBuilder &p);
void enum_components(LogLevel::value);
void dump_systemc_hierarchy(PlatformBuilder &p, Logger &l, LogLevel::value lvl);

#endif
