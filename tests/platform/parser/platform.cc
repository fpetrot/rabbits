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

RABBITS_UNIT_TEST(empty_platform)
{
    PlatformParser p("empty", get_config());

    RABBITS_TEST_ASSERT(p.get_root().empty());
    RABBITS_TEST_ASSERT(!p.get_root().is_generic());
    RABBITS_TEST_ASSERT(!p.get_root().has_parent());
}


RABBITS_UNIT_TEST(missing_descr_field)
{
    const std::string yml =
        "platforms:\n"
        "  foo:"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);

    try {
        PlatformParser p("foo", pd, get_config());
    } catch(MissingFieldParseException e) {
        RABBITS_TEST_ASSERT_EQ(e.get_field(), "description");
        return;
    }

    RABBITS_TEST_ASSERT(false);
}


RABBITS_UNIT_TEST(minimal_well_formed)
{
    const std::string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    RABBITS_TEST_ASSERT_EQ(p.get_root().get_description(), "Foo platform");
    RABBITS_TEST_ASSERT(p.get_root().empty());
    RABBITS_TEST_ASSERT(!p.get_root().is_generic());
    RABBITS_TEST_ASSERT(!p.get_root().has_parent());
}


RABBITS_UNIT_TEST(generic_platform)
{
    const std::string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    generic: true"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    RABBITS_TEST_ASSERT(p.get_root().empty());
    RABBITS_TEST_ASSERT(p.get_root().is_generic());
    RABBITS_TEST_ASSERT(!p.get_root().has_parent());
}


RABBITS_UNIT_TEST(inheriting_empty_platform)
{
    const std::string yml =
        "platforms:\n"
        "  bar:\n"
        "    description: Bar platform\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    inherit: bar"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    RABBITS_TEST_ASSERT(p.get_root().empty());
    RABBITS_TEST_ASSERT(!p.get_root().is_generic());
    RABBITS_TEST_ASSERT(p.get_root().has_parent());
    RABBITS_TEST_ASSERT_EQ(p.get_root().get_parent_name(), "bar");
}

RABBITS_UNIT_TEST(inheriting_unknown_platform)
{
    const std::string yml =
        "platforms:\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    inherit: bar"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);

    try {
        PlatformParser p("foo", pd, get_config());
    } catch (PlatformParseException e) {
        return;
    }

    RABBITS_TEST_ASSERT(false);
}

RABBITS_UNIT_TEST(inheriting_generic_platform)
{
    const std::string yml =
        "platforms:\n"
        "  bar:\n"
        "    description: Bar platform\n"
        "    generic: true\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    inherit: bar"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    RABBITS_TEST_ASSERT(p.get_root().empty());
    RABBITS_TEST_ASSERT(!p.get_root().is_generic());
    RABBITS_TEST_ASSERT(p.get_root().has_parent());
    RABBITS_TEST_ASSERT_EQ(p.get_root().get_parent_name(), "bar");
}


RABBITS_UNIT_TEST(two_level_inheriting_platform)
{
    const std::string yml =
        "platforms:\n"
        "  baz:\n"
        "    description: Baz platform\n"
        "  bar:\n"
        "    description: Bar platform\n"
        "    inherit: baz\n"
        "  foo:\n"
        "    description: Foo platform\n"
        "    inherit: bar"
        ;

    PlatformDescription pd = apply_platform(get_config(), yml);
    PlatformParser p("foo", pd, get_config());

    RABBITS_TEST_ASSERT(p.get_root().empty());
    RABBITS_TEST_ASSERT(!p.get_root().is_generic());
    RABBITS_TEST_ASSERT(p.get_root().has_parent());
    RABBITS_TEST_ASSERT_EQ(p.get_root().get_parent_name(), "bar");
}


