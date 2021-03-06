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

#ifndef _RABBITS_RABBITS_H
#define _RABBITS_RABBITS_H

#include "rabbits/config.h"
#include "component/slave.h"
#include "component/master.h"
#include "component/manager.h"
#include "config/simu.h"
#include "ui/ui.h"
#include "logger.h"
#include "plugin/plugin.h"
#include "plugin/hook.h"
#include "plugin/manager.h"
#include "platform/builder.h"
#include "platform/description.h"
#include "datatypes/address_range.h"
#include "dynloader/dynloader.h"
#include "dynloader/dynlib.h"

#include "component/port/in.h"
#include "component/port/out.h"
#include "component/port/tlm_target.h"
#include "component/port/tlm_initiator.h"
#include "component/port/vector.h"

#endif
