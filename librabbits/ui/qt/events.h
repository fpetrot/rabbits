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

#ifndef _RABBITS_LIBRABBITS_UI_QT_EVENTS_H
#define _RABBITS_LIBRABBITS_UI_QT_EVENTS_H

#include <QEvent>
#include <QString>

const QEvent::Type WEBKIT_CREATE_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);
const QEvent::Type WEBKIT_EXEC_EVENT   = static_cast<QEvent::Type>(QEvent::User + 2);

class QtUiViewWebkit;

/* Event for creating a new webkit window */
class WebkitCreateEvent : public QEvent
{
protected:
    QtUiViewWebkit *m_webkit;

public:
    WebkitCreateEvent(QtUiViewWebkit *webkit)
        : QEvent(WEBKIT_CREATE_EVENT), m_webkit(webkit)
    {}

    QtUiViewWebkit * get_webkit() { return m_webkit; }
};

/* Event for executing JS in a specific webkit window */
class WebkitExecEvent : public QEvent
{
protected:
    QtUiViewWebkit *m_webkit;
    QString m_js;

public:
    WebkitExecEvent(QtUiViewWebkit *webkit, QString js)
        : QEvent(WEBKIT_EXEC_EVENT), m_webkit(webkit), m_js(js)
    {}

    QtUiViewWebkit * get_webkit() { return m_webkit; }
    const QString & get_js() const { return m_js; }
};

#endif
