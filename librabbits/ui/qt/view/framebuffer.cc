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

#include <QVBoxLayout>
#include <QStatusBar>
#include <QLabel>

#include "framebuffer.h"

QtUiViewFramebuffer::QtUiViewFramebuffer(QWidget *parent, const std::string &name,
                                         const FramebufferInfo & info)
    : QtUiView(parent, name)
{
    m_layout = new QVBoxLayout(this);
    m_view = new Surface(this);
    m_status_lbl = new QLabel(this);

    m_status_lbl->setAlignment(Qt::AlignRight);

    m_layout->addWidget(m_view);
    set_info(info);
}

void QtUiViewFramebuffer::set_info(const FramebufferInfo & info)
{
    m_view->set_info(info);
    m_info = info;
    update_status();
}

void QtUiViewFramebuffer::set_palette(const std::vector<uint32_t> &palette)
{
    m_view->set_palette(palette);
}

void QtUiViewFramebuffer::set_backlight_level(uint8_t lvl)
{
    m_backlight_lvl = lvl;
    update_status();
}

void QtUiViewFramebuffer::enable_status()
{
    if (!m_status_enabled) {
        m_layout->addWidget(m_status_lbl);
        m_status_enabled = true;
    }
}

void QtUiViewFramebuffer::disable_status()
{
    if (m_status_enabled) {
        m_layout->removeWidget(m_status_lbl);
        m_status_enabled = false;
    }
}

void QtUiViewFramebuffer::update_status()
{
    if (!m_info.enabled) {
        enable_status();
        m_status_lbl->setText("This display is disabled");

    } else if (m_info.has_backlight) {
        enable_status();
        const int lvl = int(m_backlight_lvl) * 100 / 255;
        m_status_lbl->setText(QString("Backlight level: %1%").arg(lvl));

    } else {
        /* No need for the status label */
        disable_status();
    }
}

