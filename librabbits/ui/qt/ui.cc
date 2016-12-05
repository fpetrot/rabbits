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

#include <cassert>

#include "ui.h"

#include <semaphore.h>

#include <QApplication>
#include <QWebView>
#include <QWebFrame>
#include <QThread>
#include <QProcess>
#include <QString>
#include <QMainWindow>
#include <QMenuBar>
#include <QTabWidget>

#include "rabbits-common.h"
#include "rabbits/logger.h"

/* AUTOMOC ui.h */
#include "moc_ui.cpp"

static int _argc = 1;
static char *_argv[] = { (char *)"Rabbits" };

class MainEventFilter: public QObject
{
private:
	QTabWidget *m_tabs;

public:
    MainEventFilter(QTabWidget *tabs) : QObject()
    {
		m_tabs = tabs;
	};
    ~MainEventFilter() {};

    bool eventFilter(QObject* object, QEvent* event)
    {
        if(event->type() == WEBKIT_CREATE_EVENT) {
            WebkitCreateEvent *createEvent = (WebkitCreateEvent *)event;

            QWebView *boardView = new QWebView(m_tabs);

            m_tabs->addTab(boardView, "Board"); // TODO: platform name

            boardView->load(QUrl(QString::fromStdString(createEvent->m_webkit->m_url)));

            createEvent->m_webkit->m_view = boardView;

            QWebFrame *frame = boardView->page()->mainFrame();
            frame->addToJavaScriptWindowObject("bridge", new WebkitBridge(createEvent->m_webkit));

            return true;
        }
        else if(event->type() == WEBKIT_EXEC_EVENT) {
            WebkitExecEvent *execEvent = (WebkitExecEvent *)event;

            execEvent->m_webkit->m_view->page()->mainFrame()->evaluateJavaScript(execEvent->m_js);

            return true;
        }
        return QObject::eventFilter(object, event);
    }
};

#if 0
static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}
#endif

qt_ui::qt_ui()
{
    /* Disable GLIB in QT because it is conflicting with QEMU */
    qputenv("QT_NO_GLIB", "1");

    m_app = new QApplication(_argc, _argv);

    QApplication::setApplicationDisplayName("Rabbits"); // TODO: add platform name

    QMainWindow *window = new QMainWindow();

    QTabWidget *tabs = new QTabWidget(window);

    tabs->setMinimumSize(QSize(640, 480));

    m_app->installEventFilter(new MainEventFilter(tabs));

    QMenu *fileMenu = window->menuBar()->addMenu("&File");

    QAction *quitAction = new QAction("&Quit", window);
    fileMenu->addAction(quitAction);
    m_app->connect(quitAction, SIGNAL(triggered()), m_app, SLOT(quit()));

    window->setCentralWidget(tabs);

    window->show();
}

qt_ui::~qt_ui()
{
    delete m_app;
}

ui_fb* qt_ui::new_fb(std::string name, const ui_fb_info &info)
{
    qt_ui_fb *fb = new qt_ui_fb(info);

    m_fbs.push_back(fb);

    return fb;
}

ui_webkit* qt_ui::new_webkit(std::string url)
{
    qt_ui_webkit *webkit = new qt_ui_webkit(url);

    QApplication::postEvent(QApplication::instance(), new WebkitCreateEvent(webkit));

    return webkit;
}

void qt_ui::event_loop()
{
    QApplication::instance()->exec();
}

void qt_ui::update()
{
    // QT event loop is running in a separate pthread
}

void qt_ui::stop()
{
    QApplication::quit();
}
