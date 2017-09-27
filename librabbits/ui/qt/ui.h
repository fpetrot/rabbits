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

#include "rabbits/logger.h"
#include "rabbits/logger/wrapper.h"
#include "rabbits/ui/ui.h"

#include <QObject>

class QApplication;
class QTabWidget;
class QAction;

class QtUiView;

class TabLabelChanger : public QObject {
    Q_OBJECT

private:
    QTabWidget* m_tabs;

public:
    TabLabelChanger(QTabWidget *tabs);
    virtual ~TabLabelChanger() {}

public slots:
    void update_tab_label(const QString &);
};

class QtUi: public Ui, public HasLoggerIface
{
private:
    LoggerWrapper m_loggers;
    QApplication *m_app = nullptr;
    QTabWidget *m_tabs = nullptr;
    QAction *m_detach_action = nullptr;
    TabLabelChanger *m_tab_lbl_changer = nullptr;

    /* Custom argc/argv construction for Qt */
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
    static const int QT_ARGC = 1;
#ifdef RABBITS_WORKAROUND_CXX11_GCC_BUGS
    /* This bugs (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=43453) triggers
     * an compilation error when the array is initialized with a string literal
     * while it is allowed by the standard. It happens on GCC 4.9 and seems to
     * be fixed afterward.
     */
    char m_arg0[ARRAY_SIZE(RABBITS_APP_NAME)] = { 'r', 'a', 'b', 'b', 'i', 't', 's', '\0' };
#else
    char m_arg0[ARRAY_SIZE(RABBITS_APP_NAME)] = RABBITS_APP_NAME;
#endif
    char * m_qt_argv[QT_ARGC] { m_arg0 };
    int m_qt_argc = QT_ARGC;
#undef ARRAY_SIZE

    void set_app_name();
    void setup_window();

    static QtUi *m_inst;
    static void qt_msg_handler_entry(QtMsgType type,
                                     const QMessageLogContext &context,
                                     const QString &msg);

    void qt_msg_handler(QtMsgType type,
                        const QMessageLogContext &context,
                        const QString &msg);

    void add_tab(QtUiView *view, const std::string &name);

    void update_actions();
    void detach_current_tab();
    void reattach_tab(QtUiView *);

public:
    QtUi(ConfigManager &config);
    virtual ~QtUi();

    UiViewFramebufferIface* create_framebuffer(const std::string &name,
                                               const FramebufferInfo &info);

    UiViewWebkitIface* create_webkit(const std::string &name,
                                     const std::string &url);

    Ui::eExitStatus run();
    void stop();

    Ui::eStyle get_style() const { return Ui::GRAPHICAL; }

    /* HasLoggerIface */
    Logger & get_logger(LogContext::value context) const { return m_loggers.get_logger(context); }
};

#endif
