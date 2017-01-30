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

RABBITS_UNIT_TEST(vector0)
{
    PlatformDescription d;

    d.load_yaml("[]");

    RABBITS_TEST_ASSERT(d.is_vector());
    RABBITS_TEST_ASSERT_EQ(d.size(), 0u);

    for (auto &p: d) {
        (void) p;
        RABBITS_TEST_ASSERT(false);
    }
}

RABBITS_UNIT_TEST(vector1)
{
    PlatformDescription d;

    d.load_yaml("[ 1, foo, 5 ]");

    RABBITS_TEST_ASSERT(d.is_vector());
    RABBITS_TEST_ASSERT_EQ(d.size(), 3u);
    RABBITS_TEST_ASSERT(d[0].is_scalar());
    RABBITS_TEST_ASSERT(d[1].is_scalar());
    RABBITS_TEST_ASSERT(d[2].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[0].as<int>(), 1);
    RABBITS_TEST_ASSERT_EQ(d[1].as<std::string>(), "foo");
    RABBITS_TEST_ASSERT_EQ(d[2].as<int>(), 5);

    int loop = 0;

    for (auto &p: d) {
        switch (loop) {
        case 0:
            RABBITS_TEST_ASSERT_EQ(p.second.as<int>(), 1);
            break;
        case 1:
            RABBITS_TEST_ASSERT_EQ(p.second.as<std::string>(), "foo");
            break;
        case 2:
            RABBITS_TEST_ASSERT_EQ(p.second.as<int>(), 5);
            break;
        default:
            RABBITS_TEST_ASSERT(false);
        }
        loop++;
    }
}

RABBITS_UNIT_TEST(vector_convert0)
{
    PlatformDescription d;
    std::vector<int> vec;

    d.load_yaml("[ 1, 3, 5 ]");

    vec = d.as< std::vector<int> >();

    RABBITS_TEST_ASSERT_EQ(vec.size(), 3u);
    RABBITS_TEST_ASSERT_EQ(vec[0], 1);
    RABBITS_TEST_ASSERT_EQ(vec[1], 3);
    RABBITS_TEST_ASSERT_EQ(vec[2], 5);
}

RABBITS_UNIT_TEST(vector_convert1)
{
    PlatformDescription d;
    std::vector<int> vec;

    d.load_yaml("[ 1, foo, 5 ]");

    RABBITS_TEST_ASSERT_EXCEPTION(d.as< std::vector<int> >(), InvalidConversionException);
}
