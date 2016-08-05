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

/**
 * @file description.h
 * @brief PlatformDescription class declaration.
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
#include <algorithm>

#include "rabbits/rabbits_exception.h"
#include "rabbits/datatypes/address_range.h"

namespace YAML {
    class Node;
}

/* Data types conversion classes */
namespace platformdescription {
template <typename T> struct converter;
};

/**
 * @brief A description representing a platform.
 *
 * This data structure is used as a configuration database for Rabbits.
 * It can be fed with YAML files and the command line (and possibly more in the
 * future).
 *
 * The description contains nodes of several types (scalar, map, vector, nil
 * and invalid), with scalars being the ones carrying the values and the map
 * acting as hash tables pointing to other nodes. Hence, the final data
 * structure is roughly a tree with named children.
 * Scalar nodes can be converted to arbitrary data type using the as<T>() method.
 *
 * Given the following YAML description:
 * <pre>
 * foo:
 *   bar: 5
 * </pre>
 * in the code, one can access the "bar" value like this:
 *
 * <code>int bar_value = d["foo"]["bar"].as<int>();</code>
 */
class PlatformDescription {
public:
    typedef std::map<std::string, PlatformDescription>::iterator iterator;
    typedef std::map<std::string, PlatformDescription>::const_iterator const_iterator;
    typedef std::map<std::string, PlatformDescription>::size_type size_type;

    /**
     * @brief Possible node types.
     */
    enum NodeType {
        MAP,    /**< A node having named child nodes. */
        VECTOR, /**< A node having child nodes (not implemented). */
        SCALAR, /**< A scalar node carrying a value. */
        NIL,    /**< A empty node. */
        INVALID /**< An invalid node resulting from an erroneous description manipulation. */
    };

    /**
     * @brief Raised when converting a node to a given type is impossible.
     */
    class InvalidConversionException : public RabbitsException {
    public:
        explicit InvalidConversionException(const std::string & what) : RabbitsException(what) {}
        virtual ~InvalidConversionException() throw() {}
    };

    /**
     * @brief Raised when an error is encountered while parsing the command line.
     */
    class InvalidCmdLineException : public RabbitsException {
    protected:
        std::string build_what(const std::string & arg) { return "Invalid argument: " + arg; }
    public:
        explicit InvalidCmdLineException(const std::string & arg) : RabbitsException(build_what(arg)) {}
        virtual ~InvalidCmdLineException() throw() {}
    };

    /**
     * @brief The PlatformDescription internal node
     */
    class Node {
    public:
        struct Origin {
            enum eOrigin { FILE, CMDLINE, UNKNOWN };
            eOrigin origin = UNKNOWN;
            std::string filename;
            int line = 0;
            int column = 0;

            Origin() {}
            explicit Origin(eOrigin o) : origin(o) {}

            Origin(std::string fn, int l, int c)
                : origin(FILE), filename(fn)
                , line(l), column(c)
            {}

            std::string format() const {
                std::stringstream res;

                switch (origin) {
                case FILE:
                    res << filename;
                    break;
                case CMDLINE:
                    res << "<cmdline>";
                    break;
                case UNKNOWN:
                    res << "<unknown>";
                }

                res << ":" << line << ":" << column;
                return res.str();
            }
        };

    private:
        /* Memory management */
        int m_ref_count;
        bool m_is_static;
    protected:
        NodeType m_type;
        Origin m_origin;

    public:
        Node() : m_ref_count(0), m_is_static(false) {}
        explicit Node(const Origin &o) : m_ref_count(0), m_is_static(false), m_origin(o) {}
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
        virtual bool exists(const std::string &key) const { throw InvalidConversionException("Non-map node as no child"); }
        virtual void remove(const std::string &key) { throw InvalidConversionException("Non-map node as no child"); }

        const Origin& origin() { return m_origin; }

        /* Memory management */
        void inc_ref() { m_ref_count++; }
        void dec_ref() { m_ref_count--; }
        bool is_deletable() { return ((!m_is_static) && (m_ref_count == 0)); }
    };

    class NodeNil : public Node {
    public:
        NodeNil() : Node() {}
        explicit NodeNil(const Origin &o) : Node(o) {}

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
    public:
        NodeInvalid() : Node() {}
        explicit NodeInvalid(const Origin &o) : Node(o) {}

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

        NodeInvalid(bool is_static) : Node(is_static) {}
    };

    class NodeMap : public Node {
    protected:
        std::map<std::string, PlatformDescription> m_child;
    public:
        NodeMap() : Node() {}
        explicit NodeMap(const Origin &o) : Node(o) {}

        virtual NodeType type() const { return PlatformDescription::MAP; };
        virtual PlatformDescription& operator[] (const std::string & k) {
            return m_child[k];
        }

        virtual size_type size() const { return m_child.size(); }

        virtual bool exists(const std::string &key) const { return m_child.find(key) != m_child.end(); }

        virtual void remove(const std::string &key) { m_child.erase(key); }

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
        NodeVector() : Node() {}
        explicit NodeVector(const Origin &o) : Node(o) {}

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
        NodeScalar() : Node() {}
        explicit NodeScalar(const Origin &o) : Node(o) {}
        explicit NodeScalar(std::string val) : m_val(val) {}
        NodeScalar(std::string val, const Origin &o) : Node(o), m_val(val) {}

        virtual NodeType type() const { return PlatformDescription::SCALAR; };

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

    Node * load_yaml_req(YAML::Node root, Node::Origin &origin);

    void tokenize_arg(const std::string arg, std::list<std::string>& toks);
    NodeScalar* parse_arg_req(std::list<std::string>& toks, Node::Origin &origin);

    explicit PlatformDescription(Node *);

    void dump(std::ostream &os, int lvl) const;

public:
    PlatformDescription();
    PlatformDescription(const PlatformDescription &);
    virtual ~PlatformDescription();

    PlatformDescription& operator=(const PlatformDescription&);

    /**
     * @brief Load a YAML description into this description.
     *
     * @param[in] yaml the YAML description.
     */
    void load_yaml(const std::string & yaml);

    /**
     * @brief Load a YAML description from the given file.
     *
     * @param[in] file The path to the file containing the description.
     */
    void load_file_yaml(const std::string & file);

    /**
     * @brief Parse the command line and add the information to the description.
     *
     * This method accepts a set of unary arguments, which are arguments that
     * accept no values on the command line. They are converted to boolean
     * scalar nodes with the `true` value.
     *
     * Example:
     *   ./rabbits -help -components.foo.bar 1337
     *
     * Here, help is a unary argument, as components.foo.bar is not and as the `1337` value.
     *
     * @param[in] argc main function argc parameter.
     * @param[in] argv main function argv parameter.
     * @param[in] unaries set of unaries arguments.
     *
     * @throw InvalidCmdLineException in case of parsing error.
     */
    void parse_cmdline(int argc, const char * const argv[], const std::set<std::string> & unaries);

    /**
     * @brief Return the type of the root node.
     *
     * @return the type of the root node.
     */
    NodeType type() const { return m_node->type(); }

    /**
     * @brief Return true if the root node is a map node.
     *
     * @return true if the root node is a map node, false otherwise.
     */
    bool is_map() const { return type() == MAP; }

    /**
     * @brief Return true if the root node is a vector node.
     *
     * @return true if the root node is a vector node, false otherwise.
     */
    bool is_vector() const { return type() == VECTOR; }

    /**
     * @brief Return true if the root node is a scalar node.
     *
     * @return true if the root node is a scalar node, false otherwise.
     */
    bool is_scalar() const { return type() == SCALAR; }

    /**
     * @brief Return true if the root node is a nil node.
     *
     * @return true if the root node is a nil node, false otherwise.
     */
    bool is_nil() const { return type() == NIL; }

    /**
     * @brief Return true if the root node is a invalid node.
     *
     * @return true if the root node is a invalid node, false otherwise.
     */
    bool is_invalid() const { return type() == INVALID; }

    /**
     * @brief Return the child node associated to the given k.
     *
     * If the node type is a map, this method returns the child node associated
     * to the key k. If the key does not exist, a nil node is returned.
     *
     * If the node type is not a map, this method returns the INVALID_DESCRIPTION.
     *
     * @param[in] k the child key.
     *
     * @return the child node.
     */
    PlatformDescription & operator[] (const std::string& k) { return (*m_node)[k]; }

    /**
     * @brief Return true if the child node exists.
     *
     * If the node type is not a map, this method returns false.
     *
     * @param[in] k the key of the child node.
     *
     * @return true if the child node exists.
     */
    bool exists(const std::string& k) const {
        if (!is_map()) { return false; }

        return m_node->exists(k);
    }

    /**
     * @brief Remove the child node given by key k.
     *
     * If the node type is not a map, or if the key does not exists,
     * this method does nothing.
     *
     * @param[in] k the key to remove.
     */
    void remove(const std::string& k) {
        if (!is_map()) { return; }
        if (!exists(k)) { return; }

        m_node->remove(k);
    }

    void alias(const std::string& root, const std::string& child);

    /**
     * @brief Merge two descriptions and return the now one.
     *
     * Merge the current description with the one given as parameter.
     * The current description has priority over the one given as parameter.
     * If two nodes conflicts, the one from the current description will be
     * used in the final description.
     *
     * @param[in] p the description to merge to.
     *
     * @return the merged description.
     */
    PlatformDescription merge(PlatformDescription &p);

    /**
     * @brief Return an iterator to the first child node of a map node.
     *
     * @return an iterator to the first child node of a map node.
     */
    iterator begin() { return m_node->begin(); }

    /**
     * @brief Return an iterator to the <i>past-the-end</i> child node of a map node.
     *
     * @return an iterator to the <i>past-the-end</i> child node of a map node.
     */
    iterator end() { return m_node->end(); }

    /**
     * @brief Return a constant iterator to the first child node of a map node.
     *
     * @return a constant iterator to the first child node of a map node.
     */
    const_iterator begin() const { return m_node->begin(); }

    /**
     * @brief Return a constant iterator to the <i>past-the-end</i> child node of a map node.
     *
     * @return a constant iterator to the <i>past-the-end</i> child node of a map node.
     */
    const_iterator end() const { return m_node->end(); }

    /**
     * @brief Convert a node to the given type.
     *
     * The node must be convertible to the type T or a
     * InvalidConversionException will be thrown.
     *
     * @tparam T the conversion type.
     *
     * @return the conversion result.
     */
    template <typename T> const T as() const {
        T ret;

        if (!platformdescription::converter<T>::decode(*m_node, ret)) {
            throw InvalidConversionException("Invalid conversion");
        }

        return ret;
    }

    std::string origin() const { return m_node->origin().format(); }

    /**
     * @brief Dump the content of the description.
     *
     * Dump the content of the description using the output stream given as a
     * parameter.
     *
     * @param[in] os the stream to use for the dump.
     */
    void dump(std::ostream &os) const;

    /* The invalid description */
private:
    static NodeInvalid INVALID_NODE;
public:
    /**
     * @brief The invalid description.
     */
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

        std::string lo = n.raw_data();
        std::transform(lo.begin(), lo.end(), lo.begin(), ::tolower);

        if (lo == "true") {
            res = true;
            return true;
        }

        if (lo == "false") {
            res = false;
            return true;
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
