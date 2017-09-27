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

#ifndef _UI_QT_FB_H
#define _UI_QT_FB_H

#include "rabbits/ui/view/framebuffer.h"

#include "../view.h"
#include "../surface.h"

class QLabel;
class QLayout;

class QtUiViewFramebuffer: public QtUiView, public UiViewFramebufferIface
{
    Q_OBJECT
private:
    Surface *m_view;
    QLabel *m_status_lbl;
    QLayout *m_layout;
    FramebufferInfo m_info;

    bool m_status_enabled = false;
    bool m_has_backlight = false;
    uint8_t m_backlight_lvl = 0;

    void enable_status();
    void disable_status();
    void update_status();

public:
    QtUiViewFramebuffer(QWidget *parent, const std::string &name,
                        const FramebufferInfo & info);
    virtual ~QtUiViewFramebuffer() {}

    /* UiViewFramebufferIface */
    void set_info(const FramebufferInfo & info);
    void set_palette(const std::vector<uint32_t> &palette);
    void set_backlight_level(uint8_t lvl);

    /* UiView */
    const std::string & get_name() const { return qt_ui_get_name(); }
    void set_name(const std::string &name) { qt_ui_set_name(name); }
};

#endif
