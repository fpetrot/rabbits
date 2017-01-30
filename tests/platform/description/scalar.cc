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

RABBITS_UNIT_TEST(scalar_string)
{
    PlatformDescription d;

    d.load_yaml("foo");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<std::string>(), "foo");
}

RABBITS_UNIT_TEST(scalar_integer0)
{
    PlatformDescription d;

    d.load_yaml("0");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<int>(), 0);
    RABBITS_TEST_ASSERT_EQ(d.as<uint8_t>(), 0);
    RABBITS_TEST_ASSERT_EQ(d.as<int8_t>(), 0);
    RABBITS_TEST_ASSERT_EQ(d.as<uint16_t>(), 0);
    RABBITS_TEST_ASSERT_EQ(d.as<int16_t>(), 0);
    RABBITS_TEST_ASSERT_EQ(d.as<uint32_t>(), 0u);
    RABBITS_TEST_ASSERT_EQ(d.as<int32_t>(), 0);
    RABBITS_TEST_ASSERT_EQ(d.as<uint64_t>(), 0u);
    RABBITS_TEST_ASSERT_EQ(d.as<int64_t>(), 0);
    RABBITS_TEST_ASSERT_EQ(d.as<bool>(), false);
}

RABBITS_UNIT_TEST(scalar_integer1)
{
    PlatformDescription d;

    d.load_yaml("127");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<int>(), 127);
    RABBITS_TEST_ASSERT_EQ(d.as<uint8_t>(), 127);
    RABBITS_TEST_ASSERT_EQ(d.as<int8_t>(), 127);
    RABBITS_TEST_ASSERT_EQ(d.as<uint16_t>(), 127);
    RABBITS_TEST_ASSERT_EQ(d.as<int16_t>(), 127);
    RABBITS_TEST_ASSERT_EQ(d.as<uint32_t>(), 127u);
    RABBITS_TEST_ASSERT_EQ(d.as<int32_t>(), 127);
    RABBITS_TEST_ASSERT_EQ(d.as<uint64_t>(), 127u);
    RABBITS_TEST_ASSERT_EQ(d.as<int64_t>(), 127);

    RABBITS_TEST_ASSERT_EXCEPTION(d.as<bool>(), InvalidConversionException);
}

RABBITS_UNIT_TEST(scalar_integer2)
{
    PlatformDescription d;

    d.load_yaml("255");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<int>(), 255);
    RABBITS_TEST_ASSERT_EQ(d.as<uint8_t>(), 255);
    RABBITS_TEST_ASSERT_EQ(d.as<uint16_t>(), 255);
    RABBITS_TEST_ASSERT_EQ(d.as<int16_t>(), 255);
    RABBITS_TEST_ASSERT_EQ(d.as<uint32_t>(), 255u);
    RABBITS_TEST_ASSERT_EQ(d.as<int32_t>(), 255);
    RABBITS_TEST_ASSERT_EQ(d.as<uint64_t>(), 255u);
    RABBITS_TEST_ASSERT_EQ(d.as<int64_t>(), 255);

    RABBITS_TEST_ASSERT_EXCEPTION(d.as<bool>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<int8_t>(), InvalidConversionException);
}

RABBITS_UNIT_TEST(scalar_integer3)
{
    PlatformDescription d;

    d.load_yaml("0x1337");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<int>(), 0x1337);
    RABBITS_TEST_ASSERT_EQ(d.as<uint16_t>(), 0x1337);
    RABBITS_TEST_ASSERT_EQ(d.as<int16_t>(), 0x1337);
    RABBITS_TEST_ASSERT_EQ(d.as<uint32_t>(), 0x1337u);
    RABBITS_TEST_ASSERT_EQ(d.as<int32_t>(), 0x1337);
    RABBITS_TEST_ASSERT_EQ(d.as<uint64_t>(), 0x1337u);
    RABBITS_TEST_ASSERT_EQ(d.as<int64_t>(), 0x1337);

    RABBITS_TEST_ASSERT_EXCEPTION(d.as<bool>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<int8_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<uint8_t>(), InvalidConversionException);

}

RABBITS_UNIT_TEST(scalar_integer4)
{
    PlatformDescription d;

    d.load_yaml("-1234");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<int>(), -1234);
    RABBITS_TEST_ASSERT_EQ(d.as<int16_t>(), -1234);
    RABBITS_TEST_ASSERT_EQ(d.as<int32_t>(), -1234);
    RABBITS_TEST_ASSERT_EQ(d.as<int64_t>(), -1234);

    RABBITS_TEST_ASSERT_EXCEPTION(d.as<bool>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<int8_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<uint8_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<uint16_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<uint32_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<uint64_t>(), InvalidConversionException);
}

RABBITS_UNIT_TEST(scalar_double)
{
    PlatformDescription d;

    d.load_yaml("1234.56");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<float>(), 1234.56f);
    RABBITS_TEST_ASSERT_EQ(d.as<double>(), 1234.56);

    RABBITS_TEST_ASSERT_EXCEPTION(d.as<bool>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<int8_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<uint8_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<int16_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<uint16_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<int32_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<uint32_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<int64_t>(), InvalidConversionException);
    RABBITS_TEST_ASSERT_EXCEPTION(d.as<uint64_t>(), InvalidConversionException);
}

RABBITS_UNIT_TEST(scalar_bool0)
{
    PlatformDescription d;

    d.load_yaml("true");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<bool>(), true);
}

RABBITS_UNIT_TEST(scalar_bool1)
{
    PlatformDescription d;

    d.load_yaml("false");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<bool>(), false);
}

RABBITS_UNIT_TEST(scalar_bool2)
{
    PlatformDescription d;

    d.load_yaml("FaLsE");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<bool>(), false);
}


RABBITS_UNIT_TEST(scalar_bool3)
{
    PlatformDescription d;

    d.load_yaml("0");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<bool>(), false);
}

RABBITS_UNIT_TEST(scalar_bool4)
{
    PlatformDescription d;

    d.load_yaml("1");

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<bool>(), true);
}

