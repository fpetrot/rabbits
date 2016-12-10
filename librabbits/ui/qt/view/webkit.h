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

#include "rabbits/logger.h"
#include "rabbits/ui/view/webkit.h"

#include "../view.h"

#include <QWebView>

class QtUiViewWebkit : public QtUiView, public UiViewWebkitIface
{
protected:
    QWebView *m_view;
    std::string m_url;
    std::vector<std::string> m_updates;
    std::vector<UiWebkitEventListener*> m_listeners;

public:
    QtUiViewWebkit(const std::string &name, QApplication *app,
                   const std::string & url);

    void exec_js(const std::string & js);
    void register_event_listener(UiWebkitEventListener &l);

    void event(const std::string &ev);

    const std::string & get_url() const { return m_url; }

    void set_view(QWebView *view) { m_view = view; }
    QWebView * get_view() { return m_view; }
};

class WebkitBridge : public QObject
{
protected:
    Q_OBJECT
    QtUiViewWebkit *m_webkit;

public:
    WebkitBridge(QtUiViewWebkit *webkit) : QObject(webkit->get_view())
    {
        m_webkit = webkit;
    }

    Q_INVOKABLE void callback(QString id) {
        LOG_F(APP, DBG, "webkit-bridge: %s\n", id.toUtf8().data());
        m_webkit->event(id.toUtf8().constData());
    }
};

#endif
