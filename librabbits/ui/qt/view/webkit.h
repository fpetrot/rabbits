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

class QWebView;

class QtUiViewWebkit : public QtUiView, public UiViewWebkitIface
{
    Q_OBJECT
protected:
    QWebView *m_view = nullptr;

    std::string m_url;
    std::vector<UiWebkitEventListener*> m_listeners;

public:
    QtUiViewWebkit(QWidget *parent, const std::string &name,
                   const std::string &url);

    void exec_js(const std::string & js);
    void register_event_listener(UiWebkitEventListener &l);

    const std::string & get_url() const { return m_url; }

signals:
    void request_exec_js(const QString &);

public slots:
    void do_exec_js(const QString &);
    void js_event(const QString &);
};

class JavascripCallback : public QObject {
    Q_OBJECT
protected:
    QtUiViewWebkit *m_webkit;

public:
    JavascripCallback(QObject *parent) : QObject(parent) {}

    Q_INVOKABLE void callback(QString id) {
        LOG_F(APP, DBG, "webkit-bridge: %s\n", id.toUtf8().data());
        emit report_js_event(id);
    }

signals:
    void report_js_event(const QString &);
};

#endif
