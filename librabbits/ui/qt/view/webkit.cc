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

#include <QWebFrame>
#include <QVBoxLayout>
#include <QWebView>

QtUiViewWebkit::QtUiViewWebkit(QWidget *parent, const std::string &name,
                               const std::string & url)
    : QtUiView(parent, name), m_url(url)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_view = new QWebView;

    m_view->load(QUrl(QString::fromStdString(url)));
    layout->addWidget(m_view);

    JavascripCallback *js = new JavascripCallback(this);
    QWebFrame *frame = m_view->page()->mainFrame();
    frame->addToJavaScriptWindowObject("bridge", js);
    connect(js, SIGNAL(report_js_event(QString)), this, SLOT(js_event(QString)));

    connect(this, SIGNAL(request_exec_js(QString)), this, SLOT(do_exec_js(QString)));
}

void QtUiViewWebkit::exec_js(const std::string &js)
{
    emit request_exec_js(QString::fromStdString(js));
}

void QtUiViewWebkit::do_exec_js(const QString &js)
{
    QWebFrame * frame = m_view->page()->mainFrame();
    frame->evaluateJavaScript(js);
}

void QtUiViewWebkit::register_event_listener(UiWebkitEventListener &l)
{
    m_listeners.push_back(&l);
}

void QtUiViewWebkit::js_event(const QString &id)
{
    for (auto *l : m_listeners) {
        l->webkit_event(id.toUtf8().constData());
    }
}
