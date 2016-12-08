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

#ifndef _UI_QT_WEBKIT_H
#define _UI_QT_WEBKIT_H

#include "rabbits/ui/ui_webkit.h"

#include <QWebView>

class qt_ui_webkit : public ui_webkit
{
public:
    QWebView *m_view;

	std::string m_url;

    std::vector<std::string> m_updates;

public:
    qt_ui_webkit(std::string url);

    void exec_js(std::string js);

    void poll_updates(std::vector<std::string> &updates);
};

#endif
