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

#include "rabbits/config.h"
#include "rabbits/config/manager.h"

#include "ui.h"
#include "view/framebuffer.h"
#include "view/webkit.h"
#include "tester.h"

#include <QApplication>
#include <QWebView>
#include <QWebFrame>
#include <QThread>
#include <QProcess>
#include <QString>
#include <QMainWindow>
#include <QMenuBar>
#include <QTabWidget>

QtUi * QtUi::m_inst = nullptr;

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

    bool webkit_create(QEvent *event)
    {
        WebkitCreateEvent *create_ev;
        create_ev = reinterpret_cast<WebkitCreateEvent*>(event);
        QtUiViewWebkit *webkit = create_ev->get_webkit();

        QWebView *boardView = new QWebView(m_tabs);

        QString tab_name(QString::fromStdString(webkit->get_name()));
        m_tabs->addTab(boardView, tab_name);

        boardView->load(QUrl(QString::fromStdString(webkit->get_url())));

        webkit->set_view(boardView);

        QWebFrame *frame = boardView->page()->mainFrame();
        frame->addToJavaScriptWindowObject("bridge", new WebkitBridge(webkit));
        return true;
    }

    bool webkit_exec(QEvent *event)
    {
        WebkitExecEvent *exec_ev;
        exec_ev = reinterpret_cast<WebkitExecEvent *>(event);
        QtUiViewWebkit *webkit = exec_ev->get_webkit();
        QWebFrame * frame = webkit->get_view()->page()->mainFrame();

        frame->evaluateJavaScript(exec_ev->get_js());
        return true;
    }

    bool eventFilter(QObject *object, QEvent *event)
    {
        const QEvent::Type ev = event->type();

        if(ev == WEBKIT_CREATE_EVENT) {
            return webkit_create(event);
        } else if (ev == WEBKIT_EXEC_EVENT) {
            return webkit_exec(event);
        } else {
            return QObject::eventFilter(object, event);
        }
    }
};

void QtUi::qt_msg_handler_entry(QtMsgType type,
                                const QMessageLogContext &context,
                                const QString &msg)
{
    assert(m_inst);
    m_inst->qt_msg_handler(type, context, msg);
}

void QtUi::qt_msg_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        MLOG(APP, DBG) << localMsg.constData() << "\n";
        break;
#if QT_VERSION >= 0x050500
    case QtInfoMsg:
        MLOG(APP, INF) << localMsg.constData() << "\n";
        break;
#endif
    case QtWarningMsg:
        MLOG(APP, WRN) << localMsg.constData() << "\n";
        break;
    case QtCriticalMsg:
        MLOG(APP, ERR) << localMsg.constData() << "\n";
        break;
    case QtFatalMsg:
        MLOG(APP, ERR) << "Qt fatal error: " << localMsg.constData() << "\n";
        MLOG(APP, ERR) << "Try to run with the `-nographic' command line argument\n";
    }
}

void QtUi::set_app_name()
{
    Parameters &g = m_config.get_global_params();

    QString rabbits = RABBITS_APP_NAME;
    QString platform = g["selected-platform"].as<std::string>().c_str();

    rabbits[0] = rabbits[0].toUpper();
    platform[0] = platform[0].toUpper();

    m_app->setApplicationDisplayName(rabbits + " - " + platform);
}

void QtUi::setup_window()
{
    QMainWindow *window = new QMainWindow();

    /* Menu */
    QMenu *fileMenu = window->menuBar()->addMenu("&File");
    QAction *quitAction = new QAction("&Quit", window);
    fileMenu->addAction(quitAction);
    m_app->connect(quitAction, SIGNAL(triggered()), m_app, SLOT(quit()));

    /* Tabs */
    QTabWidget *tabs = new QTabWidget(window);
    tabs->setMinimumSize(QSize(640, 480));
    m_app->installEventFilter(new MainEventFilter(tabs));
    window->setCentralWidget(tabs);

    window->show();
}

QtUi::QtUi(ConfigManager &config)
    : Ui(config)
    , m_loggers("qt ui", config, Parameters::EMPTY, config)
{
    MLOG(APP, DBG) << "Creating Qt UI\n";

    QtTester tester;

    if (m_inst != nullptr) {
        LOG(APP, ERR) << "Trying to create multiple Qt UI\n";
        throw UiCreationFailureException("Qt UI already exists");
    }
    m_inst = this;

    if (!tester.test()) {
        /* QApplication would fail */
        throw UiCreationFailureException("Qt UI creation failure");
    }

    /* Disable GLIB in QT because it is conflicting with QEMU */
    qputenv("QT_NO_GLIB", "1");

    qInstallMessageHandler(qt_msg_handler_entry);

    m_app = new QApplication(m_qt_argc, m_qt_argv);
    set_app_name();
    setup_window();
}

QtUi::~QtUi()
{
    MLOG(APP, DBG) << "Destroying Qt UI\n";
    delete m_app;
}

UiViewFramebufferIface* QtUi::create_framebuffer(const std::string &name,
                                                 const UiFramebufferInfo &info)
{
    return nullptr;
}

UiViewWebkitIface* QtUi::create_webkit(const std::string &name,
                                       const std::string &url)
{
    QtUiViewWebkit *webkit = new QtUiViewWebkit(name, m_app, url);

    m_app->postEvent(m_app, new WebkitCreateEvent(webkit));

    return webkit;
}

Ui::eExitStatus QtUi::run()
{
    m_app->exec();
    return Ui::WANT_QUIT;
}

void QtUi::stop()
{
    m_app->quit();
}
