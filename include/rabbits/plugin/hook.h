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

#ifndef _UTILS_PLUGIN_HOOK_H
#define _UTILS_PLUGIN_HOOK_H

class PlatformDescription;
class PlatformBuilder;

class PluginHookBeforeBuild {
protected:
    PlatformDescription *m_descr;
    PlatformBuilder *m_builder;

public:
    PluginHookBeforeBuild(PlatformDescription *descr, PlatformBuilder *builder) 
        : m_descr(descr), m_builder(builder) {}

    PlatformDescription* get_descr() const { return m_descr; }
    PlatformBuilder* get_builder() const { return m_builder; }
};

class PluginHookAfterComponentDiscovery {
protected:
    PlatformDescription *m_descr;
    PlatformBuilder *m_builder;

public:
    PluginHookAfterComponentDiscovery (PlatformDescription *descr, PlatformBuilder *builder) 
        : m_descr(descr), m_builder(builder) {}

    PlatformDescription* get_descr() const { return m_descr; }
    PlatformBuilder* get_builder() const { return m_builder; }
};

class PluginHookAfterComponentInst {
protected:
    PlatformDescription *m_descr;
    PlatformBuilder *m_builder;

public:
    PluginHookAfterComponentInst (PlatformDescription *descr, PlatformBuilder *builder) 
        : m_descr(descr), m_builder(builder) {}

    PlatformDescription* get_descr() const { return m_descr; }
    PlatformBuilder* get_builder() const { return m_builder; }
};

class PluginHookAfterBusConnections {
protected:
    PlatformDescription *m_descr;
    PlatformBuilder *m_builder;

public:
    PluginHookAfterBusConnections (PlatformDescription *descr, PlatformBuilder *builder) 
        : m_descr(descr), m_builder(builder) {}

    PlatformDescription* get_descr() const { return m_descr; }
    PlatformBuilder* get_builder() const { return m_builder; }
};

class PluginHookAfterBuild {
protected:
    PlatformDescription *m_descr;
    PlatformBuilder *m_builder;

public:
    PluginHookAfterBuild(PlatformDescription *descr, PlatformBuilder *builder) 
        : m_descr(descr), m_builder(builder) {}

    PlatformDescription* get_descr() const { return m_descr; }
    PlatformBuilder* get_builder() const { return m_builder; }
};

#endif
