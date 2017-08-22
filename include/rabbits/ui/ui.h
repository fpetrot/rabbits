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

#ifndef _RABBITS_UI_UI_H
#define _RABBITS_UI_UI_H

#include <string>

#include "view/framebuffer.h"
#include "view/webkit.h"
#include "rabbits/rabbits_exception.h"

class UiCreationFailureException : public RabbitsException {
public:
    UiCreationFailureException(const std::string &what)
        : RabbitsException(what) {}
    virtual ~UiCreationFailureException() throw() {}
};

class ConfigManager;

class Ui {
protected:
    ConfigManager & m_config;

public:
    enum eExitStatus {
        CONTINUE, WANT_QUIT
    };

    Ui(ConfigManager &config) : m_config(config) {}
    virtual ~Ui() {}

    virtual UiViewFramebufferIface* create_framebuffer(const std::string &name,
                                                       const FramebufferInfo &info) = 0;

    virtual UiViewWebkitIface* create_webkit(const std::string &name,
                                             const std::string &url) = 0;

    virtual eExitStatus run() = 0;
    virtual void stop() = 0;
};

#endif
