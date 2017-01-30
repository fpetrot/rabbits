/*
 *  This file is part of Rabbits
 *  Copyright (C) 2017  Clement Deschamps and Luc Michel
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

#define RABBITS_TEST_MOD platform_description

#include <rabbits/test/test.h>

#include <rabbits/platform/description.h>

typedef PlatformDescription::InvalidConversionException InvalidConversionException;

RABBITS_UNIT_TEST(map0)
{
    PlatformDescription d;

    d.load_yaml("foo: bar");

    RABBITS_TEST_ASSERT(d.is_map());
    RABBITS_TEST_ASSERT(d["foo"].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["foo"].as<std::string>(), "bar");

    int loop = 0;

    for (auto &p : d) {
        loop++;
        RABBITS_TEST_ASSERT_EQ(p.first, "foo");
        RABBITS_TEST_ASSERT(p.second.is_scalar());
        RABBITS_TEST_ASSERT_EQ(p.second.as<std::string>(), "bar");
    }

    RABBITS_TEST_ASSERT_EQ(loop, 1);
}

RABBITS_UNIT_TEST(map1)
{
    PlatformDescription d;

    d.load_yaml(
                "foo: bar\n"
                "miou: 2"
               );

    RABBITS_TEST_ASSERT(d.is_map());
    RABBITS_TEST_ASSERT(d["foo"].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["foo"].as<std::string>(), "bar");

    int loop = 0;

    for (auto &p : d) {
        switch (loop) {
        case 0:
            RABBITS_TEST_ASSERT_EQ(p.first, "foo");
            RABBITS_TEST_ASSERT(p.second.is_scalar());
            RABBITS_TEST_ASSERT_EQ(p.second.as<std::string>(), "bar");
            break;

        case 1:
            RABBITS_TEST_ASSERT_EQ(p.first, "miou");
            RABBITS_TEST_ASSERT(p.second.is_scalar());
            RABBITS_TEST_ASSERT_EQ(p.second.as<int>(), 2);
            break;

        default:
            RABBITS_TEST_ASSERT(false);
        }

        loop++;
    }

    RABBITS_TEST_ASSERT_EQ(loop, 2);
}

RABBITS_UNIT_TEST(map2)
{
    PlatformDescription d;

    d.load_yaml(
                "foo:\n"
                "  miou: 2\n"
                "bar:\n"
                "  miou: 4"
               );

    RABBITS_TEST_ASSERT(d.is_map());
    RABBITS_TEST_ASSERT(d["foo"].is_map());
    RABBITS_TEST_ASSERT(d["bar"].is_map());
    RABBITS_TEST_ASSERT(d["foo"]["miou"].is_scalar());
    RABBITS_TEST_ASSERT(d["bar"]["miou"].is_scalar());
}
