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
#include "tester.h"

#ifdef RABBITS_CONFIG_QT_FRAMEBUFFER
#include "view/framebuffer.h"
#endif
#include "view/webkit.h"

#include <QApplication>
#include <QWebView>
#include <QWebFrame>
#include <QThread>
#include <QProcess>
#include <QString>
#include <QMainWindow>
#include <QMenuBar>
#include <QTabWidget>
#include <QCloseEvent>

QtUi * QtUi::m_inst = nullptr;

/* MainWindow: override default close behavior */
class MainWindow : public QMainWindow {
protected:
    /* Force quit when main window is closed */
    void closeEvent(QCloseEvent *event) {
        event->accept();
        QApplication::quit();
    }
};


/* TabLabelChanger: QObject that reacts to QtUiView name_changed signals */
TabLabelChanger::TabLabelChanger(QTabWidget *tabs)
    : QObject(tabs), m_tabs(tabs)
{}

void TabLabelChanger::update_tab_label(const QString &lbl)
{
    assert(m_tabs);

    QWidget *w = dynamic_cast<QWidget*>(sender());

    if (w == nullptr) {
        return;
    }

    int idx = m_tabs->indexOf(w);

    if (idx == -1) {
        return;
    }

    m_tabs->setTabText(idx, lbl);
}


/* QtUi: Main Rabbits Ui class for Qt */
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

void QtUi::update_actions()
{
    m_detach_action->setEnabled(m_tabs->count() > 1);
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

void QtUi::reattach_tab(QtUiView *v)
{
    LOG(APP, INF) << "attach\n";
    v->hide();
    v->setParent(m_tabs);
    m_tabs->addTab(v, QString::fromStdString(v->qt_ui_get_name()));
    v->show();

    update_actions();
}

void QtUi::detach_current_tab()
{
    QWidget *w = m_tabs->currentWidget();

    if (w == nullptr) {
        return;
    }

    QtUiView *v = dynamic_cast<QtUiView *>(w);

    assert(v);

    m_tabs->removeTab(m_tabs->indexOf(v));
    v->hide();
    v->setParent(nullptr);
    v->show();

    m_app->connect(v, &QtUiView::window_closed,
        [this, v] () {
            this->reattach_tab(v);
        } );

    update_actions();
}

void QtUi::setup_window()
{
    QMainWindow *window = new MainWindow();

    /* Menu */
    QMenu *fileMenu = window->menuBar()->addMenu("&File");
    QAction *quitAction = new QAction("&Quit", window);
    fileMenu->addAction(quitAction);
    m_app->connect(quitAction, SIGNAL(triggered()), m_app, SLOT(quit()));

    QMenu *viewMenu = window->menuBar()->addMenu("&View");
    m_detach_action = new QAction("&Detach", window);
    viewMenu->addAction(m_detach_action);
    m_app->connect(m_detach_action, &QAction::triggered,
        [this] () {
            this->detach_current_tab();
        } );

    /* Tabs */
    m_tabs = new QTabWidget(window);
    m_tabs->setMinimumSize(QSize(640, 480));
    window->setCentralWidget(m_tabs);

    m_tab_lbl_changer = new TabLabelChanger(m_tabs);

    update_actions();
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

void QtUi::add_tab(QtUiView *view, const std::string &name)
{
    m_tabs->addTab(view, QString::fromStdString(name));
    m_tab_lbl_changer->connect(view, &QtUiView::name_changed,
                               m_tab_lbl_changer, &TabLabelChanger::update_tab_label);
    view->show();
    update_actions();
}

UiViewFramebufferIface* QtUi::create_framebuffer(const std::string &name,
                                                 const FramebufferInfo &info)
{
#ifdef RABBITS_CONFIG_QT_FRAMEBUFFER
    QtUiViewFramebuffer *fb = new QtUiViewFramebuffer(nullptr, name, info);
    add_tab(fb, name);

    return fb;
#else
    return nullptr;
#endif
}

UiViewWebkitIface* QtUi::create_webkit(const std::string &name,
                                       const std::string &url)
{
    QtUiViewWebkit *webkit = new QtUiViewWebkit(nullptr, name, url);
    add_tab(webkit, name);

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
