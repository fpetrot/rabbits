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

using std::string;

typedef PlatformDescription::InvalidConversionException InvalidConversionException;

RABBITS_UNIT_TEST(json_parsing_error) {
    const char * json = "bad json";

    PlatformDescription d;

    RABBITS_TEST_ASSERT_EXCEPTION(d.load_json(json), PlatformDescription::JsonParsingException);
}

RABBITS_UNIT_TEST(json_null) {
    const char * json = "null";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d.is_nil());
}

RABBITS_UNIT_TEST(json_scalar_bool) {
    const char * json = "true";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<bool>(), true);
}

RABBITS_UNIT_TEST(json_scalar_integer) {
    const char * json = "-1337";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<int>(), -1337);
}

RABBITS_UNIT_TEST(json_scalar_unsigned_integer) {
    const char * json = "1337";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<unsigned int>(), 1337u);
}

RABBITS_UNIT_TEST(json_scalar_double) {
    const char * json = "3.14";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<double>(), 3.14);
}

RABBITS_UNIT_TEST(json_scalar_string) {
    const char * json = "\"foo\"";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d.is_scalar());
    RABBITS_TEST_ASSERT_EQ(d.as<string>(), "foo");
}

RABBITS_UNIT_TEST(json_object_empty) {
    const char * json = "{}";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d.is_map());
    RABBITS_TEST_ASSERT_EQ(d.size(), 0u);
}

RABBITS_UNIT_TEST(json_object) {
    const char * json = "{\"a\": \"b\", \"c\": \"d\"}";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d.is_map());
    RABBITS_TEST_ASSERT_EQ(d.size(), 2u);
    RABBITS_TEST_ASSERT(d["a"].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["a"].as<string>(), "b");
    RABBITS_TEST_ASSERT_EQ(d["c"].as<string>(), "d");
}

RABBITS_UNIT_TEST(json_array_empty) {
    const char * json = "[]";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d.is_vector());
    RABBITS_TEST_ASSERT_EQ(d.size(), 0u);
}

RABBITS_UNIT_TEST(json_array) {
    const char * json = "[\"foo\", 1337, -1337, 3.14, true, false, null]";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d.is_vector());
    RABBITS_TEST_ASSERT_EQ(d.size(), 7u);

    RABBITS_TEST_ASSERT(d[0].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[0].as<string>(), "foo");

    RABBITS_TEST_ASSERT(d[1].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[1].as<unsigned int>(), 1337u);

    RABBITS_TEST_ASSERT(d[2].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[2].as<int>(), -1337);

    RABBITS_TEST_ASSERT(d[3].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[3].as<double>(), 3.14);

    RABBITS_TEST_ASSERT(d[4].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[4].as<bool>(), true);

    RABBITS_TEST_ASSERT(d[5].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[5].as<bool>(), false);

    RABBITS_TEST_ASSERT(d[6].is_nil());
}

RABBITS_UNIT_TEST(json_array_nested)
{
    const char * json = "[ [], [\"foo\", 1337], [[[-1337, 3.14, true, false, null]]]]";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d.is_vector());
    RABBITS_TEST_ASSERT_EQ(d.size(), 3u);

    RABBITS_TEST_ASSERT(d[0].is_vector());
    RABBITS_TEST_ASSERT_EQ(d[0].size(), 0u);


    RABBITS_TEST_ASSERT(d[1].is_vector());
    RABBITS_TEST_ASSERT_EQ(d[1].size(), 2u);

    RABBITS_TEST_ASSERT(d[1][0].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[1][0].as<string>(), "foo");

    RABBITS_TEST_ASSERT(d[1][1].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[1][1].as<unsigned int>(), 1337u);


    RABBITS_TEST_ASSERT(d[2].is_vector());
    RABBITS_TEST_ASSERT_EQ(d[2].size(), 1u);

    RABBITS_TEST_ASSERT(d[2][0].is_vector());
    RABBITS_TEST_ASSERT_EQ(d[2][0].size(), 1u);

    RABBITS_TEST_ASSERT(d[2][0][0].is_vector());
    RABBITS_TEST_ASSERT_EQ(d[2][0][0].size(), 5u);

    RABBITS_TEST_ASSERT(d[2][0][0][0].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[2][0][0][0].as<int>(), -1337);

    RABBITS_TEST_ASSERT(d[2][0][0][1].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[2][0][0][1].as<double>(), 3.14);

    RABBITS_TEST_ASSERT(d[2][0][0][2].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[2][0][0][2].as<bool>(), true);

    RABBITS_TEST_ASSERT(d[2][0][0][3].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d[2][0][0][3].as<bool>(), false);

    RABBITS_TEST_ASSERT(d[2][0][0][4].is_nil());
}

RABBITS_UNIT_TEST(json_mix)
{
    const char * json =
        "{"
        "   \"foo\": "
        "       ["
        "           [],"
        "           [\"foo\", 1337],"
        "           [[[-1337, 3.14, true, false, null]]]"
        "       ],"
        "   \"bar\": true,"
        "   \"baz\": 1234,"
        "   \"fru\": null,"
        "   \"miou\": {"
        "       \"orange\": \"blue\","
        "       \"green\": \"yellow\""
        "   },"
        "   \"piou\": {}"
        "}";

    PlatformDescription d;
    d.load_json(json);

    RABBITS_TEST_ASSERT(d["foo"].is_vector());
    RABBITS_TEST_ASSERT_EQ(d["foo"].size(), 3u);

    RABBITS_TEST_ASSERT(d["foo"][0].is_vector());
    RABBITS_TEST_ASSERT_EQ(d["foo"][0].size(), 0u);


    RABBITS_TEST_ASSERT(d["foo"][1].is_vector());
    RABBITS_TEST_ASSERT_EQ(d["foo"][1].size(), 2u);

    RABBITS_TEST_ASSERT(d["foo"][1][0].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["foo"][1][0].as<string>(), "foo");

    RABBITS_TEST_ASSERT(d["foo"][1][1].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["foo"][1][1].as<unsigned int>(), 1337u);


    RABBITS_TEST_ASSERT(d["foo"][2].is_vector());
    RABBITS_TEST_ASSERT_EQ(d["foo"][2].size(), 1u);

    RABBITS_TEST_ASSERT(d["foo"][2][0].is_vector());
    RABBITS_TEST_ASSERT_EQ(d["foo"][2][0].size(), 1u);

    RABBITS_TEST_ASSERT(d["foo"][2][0][0].is_vector());
    RABBITS_TEST_ASSERT_EQ(d["foo"][2][0][0].size(), 5u);

    RABBITS_TEST_ASSERT(d["foo"][2][0][0][0].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["foo"][2][0][0][0].as<int>(), -1337);

    RABBITS_TEST_ASSERT(d["foo"][2][0][0][1].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["foo"][2][0][0][1].as<double>(), 3.14);

    RABBITS_TEST_ASSERT(d["foo"][2][0][0][2].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["foo"][2][0][0][2].as<bool>(), true);

    RABBITS_TEST_ASSERT(d["foo"][2][0][0][3].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["foo"][2][0][0][3].as<bool>(), false);

    RABBITS_TEST_ASSERT(d["foo"][2][0][0][4].is_nil());


    RABBITS_TEST_ASSERT(d["bar"].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["bar"].as<bool>(), true);

    RABBITS_TEST_ASSERT(d["baz"].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["baz"].as<unsigned int>(), 1234u);

    RABBITS_TEST_ASSERT(d["fru"].is_nil());

    RABBITS_TEST_ASSERT(d["miou"].is_map());
    RABBITS_TEST_ASSERT(d["miou"]["orange"].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["miou"]["orange"].as<string>(), "blue");
    RABBITS_TEST_ASSERT(d["miou"]["green"].is_scalar());
    RABBITS_TEST_ASSERT_EQ(d["miou"]["green"].as<string>(), "yellow");

    RABBITS_TEST_ASSERT(d["piou"].is_map());
    RABBITS_TEST_ASSERT_EQ(d["piou"].size(), 0u);
}
