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

#include <cassert>

#include "ui.h"
#include "rabbits/logger.h"

DummyUi::DummyUi(ConfigManager &config)
    : Ui(config)
{
    LOG(APP, DBG) << "Dummy ui created\n";
}

DummyUi::~DummyUi()
{
    LOG(APP, DBG) << "Destroying dummy ui\n";
}

UiViewFramebufferIface* DummyUi::create_framebuffer(const std::string &name,
                                                    const FramebufferInfo &info)
{
    return nullptr;
}

UiViewWebkitIface* DummyUi::create_webkit(const std::string &name,
                                          const std::string &url)
{
    return nullptr;
}

Ui::eExitStatus DummyUi::run()
{
    return Ui::CONTINUE;
}

void DummyUi::stop()
{
}
