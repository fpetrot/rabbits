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

#include "common.h"


RABBITS_UNIT_TEST(empty_bindings)
{
    REGISTER_FOO_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    std::shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");

    RABBITS_TEST_ASSERT(c->get_bindings().empty());
}


RABBITS_UNIT_TEST(binding_comp_invalid_peer)
{
    REGISTER_FOO_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: bar"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);

    try {
        PlatformParser p("foo", pd, get_config());
    } catch (ModuleNotFoundParseException e) {
        return;
    }

    RABBITS_TEST_ASSERT(false);
}


RABBITS_UNIT_TEST(binding_comp_implicit_port_syntax1)
{
    REGISTER_FOO_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: c1\n"
        "      c1:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");

    RABBITS_TEST_ASSERT_EQ(c->get_bindings().size(), 1u);
    RABBITS_TEST_ASSERT(c->binding_exists("port-a"));

    shared_ptr<ParserNodeBinding> b = c->get_bindings()["port-a"];

    RABBITS_TEST_ASSERT_EQ(b->get_local_port_name(), "port-a");
    RABBITS_TEST_ASSERT(b->peer_port_is_implicit());
    RABBITS_TEST_ASSERT_EQ(&(b->get_peer()), p.get_root().get_component("c1").get());
}


RABBITS_UNIT_TEST(binding_comp_implicit_port_syntax2)
{
    REGISTER_FOO_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a:\n"
        "            peer: c1\n"
        "      c1:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");

    RABBITS_TEST_ASSERT_EQ(c->get_bindings().size(), 1u);
    RABBITS_TEST_ASSERT(c->binding_exists("port-a"));

    shared_ptr<ParserNodeBinding> b = c->get_bindings()["port-a"];

    RABBITS_TEST_ASSERT_EQ(b->get_local_port_name(), "port-a");
    RABBITS_TEST_ASSERT(b->peer_port_is_implicit());
    RABBITS_TEST_ASSERT_EQ(&(b->get_peer()), p.get_root().get_component("c1").get());
}


RABBITS_UNIT_TEST(binding_comp_explicit_port_syntax1)
{
    REGISTER_FOO_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: c1.port-b\n"
        "      c1:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");

    RABBITS_TEST_ASSERT_EQ(c->get_bindings().size(), 1u);
    RABBITS_TEST_ASSERT(c->binding_exists("port-a"));

    shared_ptr<ParserNodeBinding> b = c->get_bindings()["port-a"];

    RABBITS_TEST_ASSERT_EQ(b->get_local_port_name(), "port-a");
    RABBITS_TEST_ASSERT(!b->peer_port_is_implicit());
    RABBITS_TEST_ASSERT_EQ(&(b->get_peer()), p.get_root().get_component("c1").get());
    RABBITS_TEST_ASSERT_EQ(b->get_peer_port_name(), "port-b");
}


RABBITS_UNIT_TEST(binding_comp_explicit_port_syntax2)
{
    REGISTER_FOO_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a:\n"
        "            peer: c1.port-b\n"
        "      c1:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");

    RABBITS_TEST_ASSERT_EQ(c->get_bindings().size(), 1u);
    RABBITS_TEST_ASSERT(c->binding_exists("port-a"));

    shared_ptr<ParserNodeBinding> b = c->get_bindings()["port-a"];

    RABBITS_TEST_ASSERT_EQ(b->get_local_port_name(), "port-a");
    RABBITS_TEST_ASSERT(!b->peer_port_is_implicit());
    RABBITS_TEST_ASSERT_EQ(&(b->get_peer()), p.get_root().get_component("c1").get());
    RABBITS_TEST_ASSERT_EQ(b->get_peer_port_name(), "port-b");
}


RABBITS_UNIT_TEST(binding_comp_explicit_ns)
{
    REGISTER_FOO_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: components:c1\n"
        "      c1:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");

    RABBITS_TEST_ASSERT_EQ(c->get_bindings().size(), 1u);
    RABBITS_TEST_ASSERT(c->binding_exists("port-a"));

    shared_ptr<ParserNodeBinding> b = c->get_bindings()["port-a"];

    RABBITS_TEST_ASSERT_EQ(b->get_local_port_name(), "port-a");
    RABBITS_TEST_ASSERT(b->peer_port_is_implicit());
    RABBITS_TEST_ASSERT_EQ(&(b->get_peer()), p.get_root().get_component("c1").get());
}


RABBITS_UNIT_TEST(binding_comp_explicit_ns_port)
{
    REGISTER_FOO_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: components:c1.port-b\n"
        "      c1:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");

    RABBITS_TEST_ASSERT_EQ(c->get_bindings().size(), 1u);
    RABBITS_TEST_ASSERT(c->binding_exists("port-a"));

    shared_ptr<ParserNodeBinding> b = c->get_bindings()["port-a"];

    RABBITS_TEST_ASSERT_EQ(b->get_local_port_name(), "port-a");
    RABBITS_TEST_ASSERT(!b->peer_port_is_implicit());
    RABBITS_TEST_ASSERT_EQ(&(b->get_peer()), p.get_root().get_component("c1").get());
    RABBITS_TEST_ASSERT_EQ(b->get_peer_port_name(), "port-b");
}


RABBITS_UNIT_TEST(binding_comp_inst_implicit_no_port)
{
    REGISTER_FOO_COMPONENT();
    REGISTER_BAR_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: c1\n"
        "      c1:\n"
        "        type: bar\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    ComponentBase *comp0 = create_comp_foo(get_config(), "c0");
    ComponentBase *comp1 = create_comp_bar(get_config(), "c1");

    RABBITS_TEST_ASSERT(comp0 != nullptr);
    RABBITS_TEST_ASSERT(comp1 != nullptr);

    shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");
    c->set_inst(comp0);

    p.get_root().get_component("c1")->set_inst(comp1);

    try {
        p.instanciation_done();
    } catch (NoPortFoundParseException e) {
        delete comp0;
        delete comp1;
        return;
    }

    RABBITS_TEST_ASSERT(false);
}


RABBITS_UNIT_TEST(binding_comp_inst_unknown_port)
{
    REGISTER_FOO_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: c1.non-existant\n"
        "      c1:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    ComponentBase *comp0 = create_comp_foo(get_config(), "c0");
    ComponentBase *comp1 = create_comp_foo(get_config(), "c1");

    RABBITS_TEST_ASSERT(comp0 != nullptr);
    RABBITS_TEST_ASSERT(comp1 != nullptr);

    shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");
    c->set_inst(comp0);

    p.get_root().get_component("c1")->set_inst(comp1);

    try {
        p.instanciation_done();
    } catch (PortNotFoundParseException e) {
        delete comp0;
        delete comp1;
        return;
    }

    RABBITS_TEST_ASSERT(false);
}


RABBITS_UNIT_TEST(binding_comp_inst_implicit)
{
    REGISTER_FOO_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: c1\n"
        "      c1:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    ComponentBase *comp0 = create_comp_foo(get_config(), "c0");
    ComponentBase *comp1 = create_comp_foo(get_config(), "c1");

    RABBITS_TEST_ASSERT(comp0 != nullptr);
    RABBITS_TEST_ASSERT(comp1 != nullptr);

    shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");
    c->set_inst(comp0);

    p.get_root().get_component("c1")->set_inst(comp1);
    p.instanciation_done();

    RABBITS_TEST_ASSERT_EQ(c->get_bindings().size(), 1u);
    RABBITS_TEST_ASSERT(c->binding_exists("port-a"));

    shared_ptr<ParserNodeBinding> b = c->get_bindings()["port-a"];

    RABBITS_TEST_ASSERT_EQ(&(b->get_local_port()), &(comp0->get_port("port-a")));
    RABBITS_TEST_ASSERT_EQ(&(b->get_peer_port()), &(comp1->get_port("port-a")));

    delete comp0;
    delete comp1;
}


RABBITS_UNIT_TEST(binding_comp_inst_explicit)
{
    REGISTER_FOO_COMPONENT();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: c1.port-b\n"
        "      c1:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    ComponentBase *comp0 = create_comp_foo(get_config(), "c0");
    ComponentBase *comp1 = create_comp_foo(get_config(), "c1");

    RABBITS_TEST_ASSERT(comp0 != nullptr);
    RABBITS_TEST_ASSERT(comp1 != nullptr);

    shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");
    c->set_inst(comp0);

    p.get_root().get_component("c1")->set_inst(comp1);
    p.instanciation_done();

    RABBITS_TEST_ASSERT_EQ(c->get_bindings().size(), 1u);
    RABBITS_TEST_ASSERT(c->binding_exists("port-a"));

    shared_ptr<ParserNodeBinding> b = c->get_bindings()["port-a"];

    RABBITS_TEST_ASSERT_EQ(&(b->get_local_port()), &(comp0->get_port("port-a")));
    RABBITS_TEST_ASSERT_EQ(&(b->get_peer_port()), &(comp1->get_port("port-b")));

    delete comp0;
    delete comp1;
}

RABBITS_UNIT_TEST(binding_backend)
{
    REGISTER_FOO_BACKEND();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    backends:\n"
        "      b0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: b1\n"
        "      b1:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    shared_ptr<ParserNodeBackend> c = p.get_root().get_backend("b0");

    RABBITS_TEST_ASSERT_EQ(c->get_bindings().size(), 1u);
    RABBITS_TEST_ASSERT(c->binding_exists("port-a"));

    shared_ptr<ParserNodeBinding> b = c->get_bindings()["port-a"];

    RABBITS_TEST_ASSERT_EQ(b->get_local_port_name(), "port-a");
    RABBITS_TEST_ASSERT(b->peer_port_is_implicit());
    RABBITS_TEST_ASSERT_EQ(&(b->get_peer()), p.get_root().get_backend("b1").get());
}


RABBITS_UNIT_TEST(binding_comp_backend)
{
    REGISTER_FOO_COMPONENT();
    REGISTER_FOO_BACKEND();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: backends:b0.port-b\n"
        "    backends:\n"
        "      b0:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    shared_ptr<ParserNodeComponent> c = p.get_root().get_component("c0");

    RABBITS_TEST_ASSERT_EQ(c->get_bindings().size(), 1u);
    RABBITS_TEST_ASSERT(c->binding_exists("port-a"));

    shared_ptr<ParserNodeBinding> b = c->get_bindings()["port-a"];

    RABBITS_TEST_ASSERT_EQ(b->get_local_port_name(), "port-a");
    RABBITS_TEST_ASSERT(!b->peer_port_is_implicit());
    RABBITS_TEST_ASSERT_EQ(b->get_peer_port_name(), "port-b");
    RABBITS_TEST_ASSERT_EQ(&(b->get_peer()), p.get_root().get_backend("b0").get());
}

/*
 * Plugins have no bindings.
 * This test checks that the parser raise an error as soon as we try to bind a
 * port to a plugin.
 */
RABBITS_UNIT_TEST(binding_comp_plugin)
{
    REGISTER_FOO_COMPONENT();
    REGISTER_FOO_PLUGIN();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    components:\n"
        "      c0:\n"
        "        type: foo\n"
        "        bindings:\n"
        "          port-a: plugins:p0.port-b\n"
        "    plugins:\n"
        "      p0:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);

    try {
        PlatformParser p("foo", pd, get_config());
    } catch (ModuleNotFoundParseException e) {
        return;
    }

    RABBITS_TEST_ASSERT(false);
}
