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

#ifndef _RABBITS_TESTS_PLATFORM_PARSER_COMMON_H
#define _RABBITS_TESTS_PLATFORM_PARSER_COMMON_H

#include <rabbits/test/test.h>

#include <rabbits/platform/parser.h>
#include <rabbits/component/component.h>
#include <rabbits/component/factory.h>
#include <rabbits/component/port/in.h>
#include <rabbits/component/port/out.h>
#include <rabbits/backend/factory.h>
#include <rabbits/plugin/factory.h>

using std::string;
using std::shared_ptr;

static inline PlatformDescription apply_platform(ConfigManager &c,
                                                 const string &yml)
{
    c.add_yml(yml);
    return c.apply_platform("foo");
}

static inline ComponentBase * create_comp(ConfigManager &c,
                                          const string &type,
                                          const string &name)
{
    return c.get_component_manager()
        .find_by_type(type)
        ->create(name, PlatformDescription::INVALID_DESCRIPTION);
}


/* Foo component */
static inline ComponentBase * create_comp_foo(ConfigManager &c,
                                              const string &name)
{
    return create_comp(c, "foo", name);
}

class FooComponent : public Component {
public:
    FooComponent(sc_core::sc_module_name n, const Parameters &p, ConfigManager &c)
        : Component(n, p, c), p_a("port-a"), p_b("port-b")
    {}

    InPort<bool> p_a;
    OutPort<bool> p_b;
};

class FooComponentFactory : public ComponentFactory<FooComponent> {
public:
    FooComponentFactory(ConfigManager &c)
        : ComponentFactory<FooComponent>(c, "foo", "Foo component", "foo-impl", 0)
    {}
};

#define REGISTER_FOO_COMPONENT()                                \
    get_config().get_component_manager().register_factory(      \
        std::make_shared<FooComponentFactory>(get_config()));


/* Bar component */
static inline ComponentBase * create_comp_bar(ConfigManager &c,
                                              const string &name)
{
    return create_comp(c, "bar", name);
}

class BarComponent : public Component {
public:
    BarComponent(sc_core::sc_module_name n, const Parameters &p, ConfigManager &c)
        : Component(n, p, c)
    {}
};

class BarComponentFactory : public ComponentFactory<BarComponent> {
public:
    BarComponentFactory(ConfigManager &c)
        : ComponentFactory<BarComponent>(c, "bar", "Bar component", "bar-impl", 0)
    {}
};

#define REGISTER_BAR_COMPONENT()                                \
    get_config().get_component_manager().register_factory(      \
        std::make_shared<BarComponentFactory>(get_config()));


class FooBackendFactory : public BackendFactory<FooComponent> {
public:
    FooBackendFactory(ConfigManager &c)
        : BackendFactory<FooComponent>(c, "foo", "Foo backend")
    {}
};

#define REGISTER_FOO_BACKEND()                                \
    get_config().get_backend_manager().register_factory(      \
        std::make_shared<FooBackendFactory>(get_config()));

class FooPlugin : public Plugin {
public:
    FooPlugin(const string &n, const Parameters &p, ConfigManager &c)
        : Plugin(n, p, c)
    {}
};

class FooPluginFactory : public PluginFactory<FooPlugin> {
public:
    FooPluginFactory(ConfigManager &c)
        : PluginFactory<FooPlugin>(c, "foo", "Foo plugin")
    {}
};

#define REGISTER_FOO_PLUGIN()                              \
    get_config().get_plugin_manager().register_factory(    \
        std::make_shared<FooPluginFactory>(get_config()));

#endif
