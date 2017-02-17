/*
 *  This file is part of Rabbits
 *  Copyright (C) 2015  Clement Deschamps and Luc Michel
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
#ifndef _RABBITS_UI_CHOOSER_H
#define _RABBITS_UI_CHOOSER_H

#include "rabbits/config.h"

class Ui;
class ConfigManager;

class UiFactoryBase {
public:
    virtual Ui* create(ConfigManager &config) const = 0;
};

template <class UI>
class UiFactory : public UiFactoryBase {
public:
    UiFactory() {}
    Ui* create(ConfigManager &config) const { return new UI(config); }
};

class UiChooser {
public:
    enum Hint {
        AUTO, HEADLESS
    };

    enum AvailableUis {
        UI_DUMMY,
#ifdef RABBITS_CONFIG_QT
        UI_QT,
#endif
        LAST_UI
    };

    enum GraphicalUis {
#ifdef RABBITS_CONFIG_QT
        GUI_QT,
#endif
        LAST_GUI
    };

    enum HeadlessUis {
        HUI_DUMMY,
        LAST_HUI
    };

private:
    static constexpr AvailableUis m_gui_map[LAST_GUI] {
#ifdef RABBITS_CONFIG_QT
        [GUI_QT] = UI_QT,
#endif
    };
    static constexpr AvailableUis m_hui_map[LAST_HUI] {
        [HUI_DUMMY] = UI_DUMMY,
    };

    static const UiFactoryBase * const m_factories[LAST_UI];

    static Ui * try_create(AvailableUis id, ConfigManager &config);

public:
    static Ui * create_ui(Hint hint, ConfigManager &config);

    UiChooser() = delete;
    UiChooser(const UiChooser&) = delete;
};

#endif
