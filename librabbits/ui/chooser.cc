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

#include <cassert>

#include "rabbits/config.h"
#include "rabbits/ui/chooser.h"
#include "rabbits/logger.h"

#include "dummy/ui.h"
static const UiFactory<DummyUi> DUMMY_UI_FACTORY;

#ifdef RABBITS_CONFIG_QT
# include "qt/ui.h"
static const UiFactory<QtUi> QT_UI_FACTORY;
#endif


const UiFactoryBase * const UiChooser::m_factories[UiChooser::LAST_UI] {
    [UI_DUMMY] = &DUMMY_UI_FACTORY,
#ifdef RABBITS_CONFIG_QT
    [UI_QT] = &QT_UI_FACTORY,
#endif
};

constexpr UiChooser::AvailableUis UiChooser::m_gui_map[UiChooser::LAST_GUI];
constexpr UiChooser::AvailableUis UiChooser::m_hui_map[UiChooser::LAST_HUI];

Ui * UiChooser::try_create(AvailableUis ui_id, ConfigManager &config)
{
    try {
        return m_factories[ui_id]->create(config);
    } catch (UiCreationFailureException e) {
        LOG(APP, WRN) << e.what() << "\n";
        return nullptr;
    }
}

Ui * UiChooser::create_ui(Hint hint, ConfigManager &config)
{
    Ui *ret = nullptr;

    switch (hint) {
    case AUTO:
        for (int i = 0; i < LAST_GUI; i++) {
            if ((ret = try_create(m_gui_map[i], config))) {
                return ret;
            }
        }

        /* Fallthrough */
    case HEADLESS:
        for (int i = 0; i < LAST_HUI; i++) {
            if ((ret = try_create(m_hui_map[i], config))) {
                return ret;
            }
        }
    }

    /* At least the dummy ui should succeed */
    assert(false);
}
