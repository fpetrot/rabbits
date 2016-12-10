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

#include "webkit.h"
#include "../events.h"

#include <QApplication>

/* AUTOMOC webkit.h */
#include "moc_webkit.cpp"

QtUiViewWebkit::QtUiViewWebkit(const std::string &name, QApplication *app,
                               const std::string & url)
    : QtUiView(name, app), m_url(url)
{}

void QtUiViewWebkit::exec_js(const std::string & js)
{
    WebkitExecEvent *ev = new WebkitExecEvent(this, QString::fromStdString(js));
    m_app->postEvent(m_view, ev);
}

void QtUiViewWebkit::register_event_listener(UiWebkitEventListener &l)
{
    m_listeners.push_back(&l);
}

void QtUiViewWebkit::event(const std::string &ev)
{
    for (auto *l : m_listeners) {
        l->webkit_event(ev);
    }
}
