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

static pthread_t qt_thread;

static sem_t sem_qt_wait;

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

            boardView->setAttribute(Qt::WA_DeleteOnClose, true);

			m_tabs->addTab(boardView,"Board"); // TODO: Platform name

            boardView->load(QUrl(QString::fromStdString(createEvent->m_webkit->m_url)));

            QWebFrame *frame = boardView->page()->mainFrame();
            frame->addToJavaScriptWindowObject("bridge", new WebkitBridge(createEvent->m_webkit));

            createEvent->m_webkit->m_view = boardView;

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
#if 0 // removing warning messages (when closing Rabbits, QT is complaining about Timers)
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
#endif
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}

void * qt_main(void *arg)
{
    /* Disable GLIB in QT because it is conflicting with QEMU */
    qputenv("QT_NO_GLIB", "1");

    int argc = 0;
    QApplication a(argc, NULL);

	qInstallMessageHandler(messageHandler);

	QMainWindow *window = new QMainWindow();
    window->setWindowTitle(QString::fromUtf8("Rabbits")); // TODO: add platform name

	/* Menus */
    QMenu *fileMenu = window->menuBar()->addMenu("&File");

    QAction *quitAction = new QAction("&Quit", window);
    fileMenu->addAction(quitAction);
    a.connect(quitAction, SIGNAL(triggered()), &a, SLOT(quit()));

    // TODO: "View" menu to detach tabs
    // QMenu *viewMenu = window->menuBar()->addMenu("&View");

    QTabWidget *tabs = new QTabWidget();
    window->setCentralWidget(tabs);

    // TODO: only in debug mode
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    a.installEventFilter(new MainEventFilter(tabs));

	window->show();

	/* initialization done */
    sem_post(&sem_qt_wait);

    a.exec();

	// if QT has been closed from "File -> Quit"
	if(sc_core::sc_get_status() != sc_core::SC_STOPPED) {
		sc_core::sc_stop();
	}

	/* application exited */
    sem_post(&sem_qt_wait);

    return NULL;
}

qt_ui::qt_ui()
{
    sem_init(&sem_qt_wait, 0, 0);

    pthread_create(&qt_thread, NULL, qt_main, NULL);
    pthread_detach(qt_thread);

	/* wait for initialization */
    sem_wait(&sem_qt_wait);
}

qt_ui::~qt_ui()
{
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

void qt_ui::update()
{
    // QT event loop is running in a separate pthread
}

void qt_ui::stop()
{
    QApplication::quit();

	// TODO: we should use pthread_join here, but it returns EINVAL (22)
	// There is probably another thread already waiting to join with this thread
	// Using a semaphore instead.
    sem_wait(&sem_qt_wait);
}
