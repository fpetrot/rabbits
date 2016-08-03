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

#ifndef _RABBITS_PLATFORM_PARSER_IMPL_BACKEND_H
#define _RABBITS_PLATFORM_PARSER_IMPL_BACKEND_H

#include "../backend.h"

#include "rabbits/component/component.h"

inline ParserNodeBackend::ParserNodeBackend(PlatformDescription &d, const std::string &n, ParserNodePlatform &root)
    : ParserNodeModuleWithPorts(d, n, root, Namespace::get(Namespace::BACKEND))
{}

inline ParserNodeBackend:: ParserNodeBackend(const std::string &name, const std::string &type,
                                             const Parameters &params, ParserNodePlatform &root)
    : ParserNodeModuleWithPorts(name, type, params, root, Namespace::get(Namespace::BACKEND))
{}

inline ParserNodeBackend::ParserNodeBackend(ComponentBase *c, ParserNodePlatform &root)
    : ParserNodeModuleWithPorts(c, root, Namespace::get(Namespace::BACKEND)), m_inst(c)
{}

inline ParserNodeBackend::~ParserNodeBackend()
{}

inline void ParserNodeBackend::set_inst(ComponentBase *inst)
{
    m_inst = inst;
    ParserNodeModuleWithPorts::set_inst(inst);
}

#endif
