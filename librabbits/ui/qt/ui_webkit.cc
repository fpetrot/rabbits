/*
 *  This file is part of Rabbits
 *  Copyright (C) 2016  Clement Deschamps and Luc Michel
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

#include "ui_webkit.h"
#include "ui.h"

#include "rabbits-common.h"
#include "rabbits/logger.h"

#include <QApplication>

qt_ui_webkit::qt_ui_webkit() : ui_webkit()
{
}

void qt_ui_webkit::exec_js(std::string js)
{
    QApplication::postEvent(m_view, new WebkitExecEvent(this, QString::fromStdString(js)));
}

void qt_ui_webkit::poll_updates(std::vector<std::string> &updates)
{
    updates.assign(m_updates.begin(), m_updates.end());
    m_updates.clear();
}

