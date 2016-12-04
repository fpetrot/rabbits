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

#ifndef _UI_QT_UI
#define _UI_QT_UI

#include <vector>

#include <rabbits/logger.h>

#include "rabbits/ui/ui.h"
#include "ui_fb.h"
#include "ui_webkit.h"

#include <QObject>
#include <QEvent>

const QEvent::Type WEBKIT_CREATE_EVENT  = static_cast<QEvent::Type>(QEvent::User + 1);
const QEvent::Type WEBKIT_EXEC_EVENT    = static_cast<QEvent::Type>(QEvent::User + 2);

/* Event for creating a new webkit window */
class WebkitCreateEvent : public QEvent
{
public:
    qt_ui_webkit *m_webkit;

public:
    WebkitCreateEvent(qt_ui_webkit *webkit) : QEvent(WEBKIT_CREATE_EVENT)
    {
        m_webkit = webkit;
    }
};

/* Event for executing JS in a specific webkit window */
class WebkitExecEvent : public QEvent
{
public:
    qt_ui_webkit *m_webkit;
    QString m_js;

public:
    WebkitExecEvent(qt_ui_webkit *webkit, QString js) : QEvent(WEBKIT_EXEC_EVENT)
    {
        m_webkit = webkit;
        m_js = js;
    }
};

class WebkitBridge : public QObject
{
    Q_OBJECT

public:
    qt_ui_webkit *m_webkit;

    WebkitBridge(qt_ui_webkit *webkit = 0) : QObject(0)
    {
        m_webkit = webkit;
    }

    Q_INVOKABLE void callback(QString id) {
		LOG_F(APP, DBG, "webkit-bridge: %s\n", id.toUtf8().data());

        m_webkit->m_updates.push_back(std::string(id.toUtf8().data()));
    }
};

class qt_ui: public ui
{
private:
    std::vector<qt_ui_fb*> m_fbs;

protected:
    qt_ui();

public:
    friend class ui;
    virtual ~qt_ui();

    void stop();

    ui_fb* new_fb(std::string name, const ui_fb_info &info);

    ui_webkit* new_webkit(std::string url);

    void update();
};

#endif
