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

#ifndef _UTILS_PLATFORM_DESCRIPTION_H
#define _UTILS_PLATFORM_DESCRIPTION_H

#include <string>
#include <map>
#include <vector>
#include <list>
#include <sstream>
#include <limits>
#include <set>

#include "rabbits/rabbits_exception.h"
#include "rabbits/datatypes/address_range.h"

namespace YAML {
    class Node;
}

/* Data types conversion classes */
namespace platformdescription {
template <typename T> struct converter;
};

class PlatformDescription {
public:
    typedef std::map<std::string, PlatformDescription>::iterator iterator;
    typedef std::map<std::string, PlatformDescription>::const_iterator const_iterator;
    typedef std::map<std::string, PlatformDescription>::size_type size_type;

    enum NodeType { MAP, VECTOR, SCALAR, NIL, INVALID };

    class InvalidConversionException : public RabbitsException {
    public:
        explicit InvalidConversionException(const std::string & what) : RabbitsException(what) {}
        virtual ~InvalidConversionException() throw() {}
    };

    class InvalidCmdLineException : public RabbitsException {
    protected:
        std::string build_what(const std::string & arg) { return "Invalid argument: " + arg; }
    public:
        explicit InvalidCmdLineException(const std::string & arg) : RabbitsException(build_what(arg)) {}
        virtual ~InvalidCmdLineException() throw() {}
    };

    class Node {
    private:
        /* Memory management */
        int m_ref_count;
        bool m_is_static;
    protected:
        NodeType m_type;
    public:
        Node() : m_ref_count(0), m_is_static(false) {}
        explicit Node(bool is_static) : m_ref_count(0), m_is_static(is_static) {}

        virtual ~Node() {}

        virtual NodeType type() const = 0;

        virtual PlatformDescription& operator[] (const std::string & k) = 0;
        virtual size_type size() const = 0;
        virtual iterator begin() = 0;
        virtual iterator end() = 0;
        virtual const_iterator begin() const = 0;
        virtual const_iterator end() const = 0;
        virtual const std::string & raw_data() const { throw InvalidConversionException("Invalid rawdata use"); }


        /* Memory management */
        void inc_ref() { m_ref_count++; }
        void dec_ref() { m_ref_count--; }
        bool is_deletable() { return ((!m_is_static) && (m_ref_count == 0)); }
    };

    class NodeNil : public Node {
        virtual NodeType type() const { return PlatformDescription::NIL; };

        virtual PlatformDescription& operator[] (const std::string & k) {
            return PlatformDescription::INVALID_DESCRIPTION;
        }

        virtual size_type size() const { return 0; }

        virtual iterator begin() {
            throw InvalidConversionException("Cannot iterate over nil");
        }

        virtual iterator end() {
            throw InvalidConversionException("Cannot iterate over nil");
        }

        virtual const_iterator begin() const {
            throw InvalidConversionException("Cannot iterate over nil");
        }

        virtual const_iterator end() const {
            throw InvalidConversionException("Cannot iterate over nil");
        }
    };

    class NodeInvalid : public Node {
        virtual NodeType type() const { return PlatformDescription::INVALID; };

        virtual PlatformDescription& operator[] (const std::string & k) {
            return PlatformDescription::INVALID_DESCRIPTION;
        }

        virtual size_type size() const { return 0; }

        virtual iterator begin() {
            throw InvalidConversionException("Cannot iterate over invalid");
        }

        virtual iterator end() {
            throw InvalidConversionException("Cannot iterate over invalid");
        }

        virtual const_iterator begin() const {
            throw InvalidConversionException("Cannot iterate over invalid");
        }

        virtual const_iterator end() const {
            throw InvalidConversionException("Cannot iterate over invalid");
        }

    public:
        NodeInvalid(bool is_static) : Node(is_static) {}
    };

    class NodeMap : public Node {
    protected:
        std::map<std::string, PlatformDescription> m_child;
    public:
        virtual NodeType type() const { return PlatformDescription::MAP; };
        virtual PlatformDescription& operator[] (const std::string & k) {
            return m_child[k];
        }

        virtual size_type size() const { return m_child.size(); }

        virtual iterator begin() {
            return m_child.begin();
        }

        virtual iterator end() {
            return m_child.end();
        }

        virtual const_iterator begin() const {
            return m_child.begin();
        }

        virtual const_iterator end() const {
            return m_child.end();
        }
    };

    class NodeVector : public Node {
    protected:
        std::vector<PlatformDescription> m_child;
    public:
        virtual NodeType type() const { return PlatformDescription::VECTOR; };
        void push_back(const PlatformDescription &p) {m_child.push_back(p);}

        virtual PlatformDescription& operator[] (const std::string & k) {
            std::vector<PlatformDescription>::size_type n;
            std::stringstream(k) >> n;
            return m_child[n];
        }

        virtual size_type size() const { return m_child.size(); }

        virtual iterator begin() {
            throw InvalidConversionException("Cannot iterate over vector (n/i)");
        }

        virtual iterator end() {
            throw InvalidConversionException("Cannot iterate over vector (n/i)");
        }

        virtual const_iterator begin() const {
            throw InvalidConversionException("Cannot iterate over vector (n/i)");
        }

        virtual const_iterator end() const {
            throw InvalidConversionException("Cannot iterate over vector (n/i)");
        }
    };

    class NodeScalar : public Node {
    protected:
        std::string m_val;
    public:
        virtual NodeType type() const { return PlatformDescription::SCALAR; };
        NodeScalar(std::string val) : m_val(val) {}

        virtual PlatformDescription& operator[] (const std::string & k) {
            return PlatformDescription::INVALID_DESCRIPTION;
        }

        virtual size_type size() const { return 1; }

        virtual iterator begin() {
            throw InvalidConversionException("Cannot iterate over scalar");
        }

        virtual iterator end() {
            throw InvalidConversionException("Cannot iterate over scalar");
        }

        virtual const_iterator begin() const {
            throw InvalidConversionException("Cannot iterate over scalar");
        }

        virtual const_iterator end() const {
            throw InvalidConversionException("Cannot iterate over scalar");
        }

        virtual const std::string & raw_data() const { return m_val; }
	void set_raw_data(const std::string &data) { m_val = data; }
    };

protected:
    Node *m_node;

    Node * load_yaml_req(YAML::Node);
    void tokenize_arg(const std::string arg, std::list<std::string>& toks);
    NodeScalar* parse_arg_req(std::list<std::string>& toks);

    explicit PlatformDescription(Node *);

public:
    PlatformDescription();
    PlatformDescription(const PlatformDescription &);
    virtual ~PlatformDescription();

    PlatformDescription& operator=(const PlatformDescription&);

    void load_yaml(const std::string & yaml);
    void load_file_yaml(const std::string & file);

    void parse_cmdline(int argc, const char * const argv[], const std::set<std::string> & unaries);

    NodeType type() const { return m_node->type(); }

    bool is_map() const { return type() == MAP; }
    bool is_vector() const { return type() == VECTOR; }
    bool is_scalar() const { return type() == SCALAR; }
    bool is_nil() const { return type() == NIL; }
    bool is_invalid() const { return type() == INVALID; }

    PlatformDescription & operator[] (const std::string& k) { return (*m_node)[k]; }

    bool exists(const std::string& k) const {
        NodeType t = (*m_node)[k].type();
        return ((t != NIL) && (t != INVALID));
    }

    void alias(const std::string& root, const std::string& child);

    PlatformDescription merge(PlatformDescription &);

    iterator begin() { return m_node->begin(); }
    iterator end() { return m_node->end(); }

    const_iterator begin() const { return m_node->begin(); }
    const_iterator end() const { return m_node->end(); }

    /* Data access/conversion */
    template <typename T> const T as() const {
        T ret;

        if (!platformdescription::converter<T>::decode(*m_node, ret)) {
            throw InvalidConversionException("Invalid conversion");
        }

        return ret;
    }

    /* The invalid description */
private:
    static NodeInvalid INVALID_NODE;
public:
    static PlatformDescription INVALID_DESCRIPTION;
};

namespace platformdescription {

static inline int64_t unit2factor(char unit) {
    switch(unit) {
    case 'K':
        return 1024;
    case 'M':
        return 1024 * 1024;
    case 'G':
        return 1024 * 1024 * 1024;
    default:
        return 0;
    }
}

template <>
struct converter<std::string> {
    static bool decode(const PlatformDescription::Node &n, std::string &res) {
        if (n.type() != PlatformDescription::SCALAR) {
            return false;
        }

        res = n.raw_data();
        return true;
    }
};

#define NUMERICAL_CONVERTER(_type, _sign)                                \
template <>                                                              \
struct converter<_type> {                                                \
    static bool decode(const PlatformDescription::Node &n, _type &res) { \
        if (n.type() != PlatformDescription::SCALAR) {                   \
            return false;                                                \
        }                                                                \
        const std::string & input = n.raw_data();                        \
        std::stringstream ss(input);                                     \
        _sign ## int64_t t, factor = 1;                                  \
        ss.unsetf(std::ios::dec);                                        \
        if (!(ss >> t)) {                                                \
            return false;                                                \
        }                                                                \
        if (!ss.eof()) {                                                 \
            /* Unit parsing */                                           \
            char unit;                                                   \
            if((!(ss >> unit)) || (!(ss >> std::ws).eof())) {            \
                return false;                                            \
            }                                                            \
            factor = unit2factor(unit);                                  \
            if (!factor) {                                               \
                /* Invalid unit */                                       \
                return false;                                            \
            }                                                            \
        }                                                                \
        t *= factor;                                                     \
        if ((t < std::numeric_limits<_type>::min())                      \
            || (t > std::numeric_limits<_type>::max())) {                \
            /* input is out of bounds for requested conversion */        \
            return false;                                                \
        }                                                                \
        res = t;                                                         \
        return true;                                                     \
    }                                                                    \
};

NUMERICAL_CONVERTER(uint8_t, u)
NUMERICAL_CONVERTER(int8_t, )
NUMERICAL_CONVERTER(uint16_t, u)
NUMERICAL_CONVERTER(int16_t, )
NUMERICAL_CONVERTER(uint32_t, u)
NUMERICAL_CONVERTER(int32_t, )
NUMERICAL_CONVERTER(uint64_t, u)
NUMERICAL_CONVERTER(int64_t, )
#undef NUMERICAL_CONVERTER

template <>
struct converter<bool> {
    static bool decode(const PlatformDescription::Node &n, bool &res) {
        if (n.type() != PlatformDescription::SCALAR) {
            return false;
        }

        std::stringstream ss(n.raw_data());
        if (!(ss >> res)) {
            return false;
        }
        if (!ss.eof()) {
            return false;
        }

        return true;
    }
};

template <>
struct converter<AddressRange> {
    static bool decode(const PlatformDescription::Node &n, AddressRange &res) {
        PlatformDescription::const_iterator it;
        if (n.type() != PlatformDescription::MAP) {
            return false;
        }

        if (n.size() != 1) {
            return false;
        }

        for (it = n.begin(); it != n.end(); it++) {
            uint64_t begin, size;

            if(!converter<uint64_t>::decode(PlatformDescription::NodeScalar(it->first), begin)) {
               return false;
            }

            size = it->second.as<uint64_t>();

            res = AddressRange(begin, size);
        }

        return true;
    }
};

};
#endif
