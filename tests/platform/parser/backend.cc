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

RABBITS_UNIT_TEST(missing_backend_type)
{
    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    backends:\n"
        "      c0:\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);

    try {
        PlatformParser p("foo", pd, get_config());
    } catch (MissingFieldParseException e) {
        RABBITS_TEST_ASSERT_EQ(e.get_field(), "type");
        return;
    }

    RABBITS_TEST_ASSERT(false);
}


RABBITS_UNIT_TEST(unknown_backend)
{
    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    backends:\n"
        "      c0:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);

    try {
        PlatformParser p("foo", pd, get_config());
    } catch (ModuleTypeNotFoundParseException e) {
        return;
    }

    RABBITS_TEST_ASSERT(false);
}


RABBITS_UNIT_TEST(simple_backend)
{
    REGISTER_FOO_BACKEND();

    const string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    backends:\n"
        "      c0:\n"
        "        type: foo\n"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    RABBITS_TEST_ASSERT_EQ(p.get_root().get_backends().size(), 1u);
    RABBITS_TEST_ASSERT(p.get_root().backend_exists("c0"));

    std::shared_ptr<ParserNodeBackend> c = p.get_root().get_backend("c0");
    RABBITS_TEST_ASSERT_EQ(c->get_name(), "c0");
    RABBITS_TEST_ASSERT_EQ(c->get_type(), "foo");
    RABBITS_TEST_ASSERT(c->get_inst() == nullptr);
    RABBITS_TEST_ASSERT_EQ(c->get_namespace().get_id(), Namespace::BACKEND);
    RABBITS_TEST_ASSERT_EQ(&(c->get_root()), &(p.get_root()));
}
