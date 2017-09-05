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
#include <memory>

#include <systemc>

#include "rabbits/rabbits_exception.h"
#include "rabbits/datatypes/address_range.h"

namespace YAML {
    class Node;
}

class JSON;

class PlatformDescription;

/* Data types conversion classes */
namespace platformdescription {
template <typename T> struct converter;

typedef std::map<std::string, PlatformDescription> MapStorage;
typedef std::vector<PlatformDescription> VecStorage;
typedef std::pair<const std::string, PlatformDescription> IteratorValue;

template <class T> struct IteratorState;

template <>
struct IteratorState<IteratorValue> {
    MapStorage::iterator map;
    VecStorage::iterator vec;

    IteratorState() {}
    IteratorState(MapStorage::iterator it) : map(it) {}
    IteratorState(VecStorage::iterator it) : vec(it) {}
};

template <>
struct IteratorState<const IteratorValue> {
    MapStorage::const_iterator map;
    VecStorage::const_iterator vec;

    IteratorState() {}
    IteratorState(MapStorage::const_iterator it) : map(it) {}
    IteratorState(VecStorage::const_iterator it) : vec(it) {}
};

static inline void tokenize(const std::string arg, const char sep, std::list<std::string>& toks)
{
    std::istringstream ss(arg);
    std::string tok;

    while(std::getline(ss, tok, sep)) {
        toks.push_back(tok);
    }
}

}

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

    class Node;

    template <class T>
    class IteratorBase : public std::iterator<std::input_iterator_tag, platformdescription::IteratorValue> {
    private:
        const Node *m_node;
        platformdescription::IteratorState<T> m_it;
        platformdescription::MapStorage::iterator m_map_it;
        platformdescription::VecStorage::iterator m_vec_it;
        std::unique_ptr<platformdescription::IteratorValue> m_pair { new platformdescription::IteratorValue };

    public:
        IteratorBase();
        IteratorBase(const IteratorBase<T> &it);
        IteratorBase(const Node &node, platformdescription::IteratorState<T> it);

        IteratorBase& operator=(const IteratorBase<T>&);

        IteratorBase& operator++();
        IteratorBase operator++(int) {IteratorBase<T> tmp(*this); operator++(); return tmp; }

        bool operator==(const IteratorBase<T>& it);
        bool operator!=(const IteratorBase<T>& it);

        T& operator*() const;
        T* operator->() const;
    };

    typedef IteratorBase<platformdescription::IteratorValue> iterator;
    typedef IteratorBase<const platformdescription::IteratorValue> const_iterator;

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
        std::string build_what(const std::string & arg) { return "Invalid command line argument `" + arg + "`"; }
    public:
        explicit InvalidCmdLineException(const std::string & arg) : RabbitsException(build_what(arg)) {}
        virtual ~InvalidCmdLineException() throw() {}
    };

    /**
     * @brief Raised when a YAML parsing error occured
     */
    class YamlParsingException : public RabbitsException {
    public:
        explicit YamlParsingException(const std::string & what) : RabbitsException(what) {}
        virtual ~YamlParsingException() throw() {}
    };

    /**
     * @brief Raised when a JSON parsing error occured
     */
    class JsonParsingException : public RabbitsException {
    public:
        explicit JsonParsingException(const std::string & what) : RabbitsException(what) {}
        virtual ~JsonParsingException() throw() {}
    };

    class NodeVisitor {
    public:
        virtual void operator() (PlatformDescription &node,
                                 const std::vector<std::string> &names) = 0;
    };

    /**
     * @brief The PlatformDescription internal node
     */
    class Node {
    public:
        /* For scalar nodes, this hint helps serializers to convert them
         * using appropriate syntax */
        enum eDataTypeHint { INTEGER, FLOAT, BOOLEAN, STRING };

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
        Origin m_origin;
        bool m_converted = false;
        eDataTypeHint m_type_hint;

    public:
        Node() : m_ref_count(0), m_is_static(false) {}
        explicit Node(const Origin &o) : m_ref_count(0), m_is_static(false), m_origin(o) {}
        explicit Node(bool is_static) : m_ref_count(0), m_is_static(is_static) {}

        virtual ~Node() {}

        virtual NodeType type() const = 0;

        virtual PlatformDescription& operator[] (const std::string & k) = 0;
        virtual PlatformDescription& operator[] (size_type i) = 0;
        virtual size_type size() const = 0;
        virtual iterator begin() = 0;
        virtual iterator end() = 0;
        virtual const_iterator begin() const = 0;
        virtual const_iterator end() const = 0;
        virtual const std::string & raw_data() const { throw InvalidConversionException("Invalid rawdata use"); }
        virtual bool exists(const std::string &key) const { throw InvalidConversionException("Non-map node has no child"); }
        virtual void remove(const std::string &key) { throw InvalidConversionException("Non-map node has no child"); }

        virtual eDataTypeHint get_type_hint() const { throw InvalidConversionException("No type hint for non-scalar node"); }

        const Origin& origin() { return m_origin; }

        void mark_converted() { m_converted = true; }
        bool has_been_converted() { return m_converted; }

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

        virtual PlatformDescription& operator[] (size_type i) {
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

        virtual PlatformDescription& operator[] (size_type i) {
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
        platformdescription::MapStorage m_child;
    public:
        NodeMap() : Node() {}
        explicit NodeMap(const Origin &o) : Node(o) {}

        virtual NodeType type() const { return PlatformDescription::MAP; };

        virtual PlatformDescription& operator[] (const std::string & k) {
            return m_child[k];
        }

        virtual PlatformDescription& operator[] (size_type i) {
            return PlatformDescription::INVALID_DESCRIPTION;
        }

        virtual size_type size() const { return m_child.size(); }

        virtual bool exists(const std::string &key) const { return m_child.find(key) != m_child.end(); }

        virtual void remove(const std::string &key) { m_child.erase(key); }

        virtual iterator begin() {
            return iterator(*this, m_child.begin());
        }

        virtual iterator end() {
            return iterator(*this, m_child.end());
        }

        virtual const_iterator begin() const {
            return const_iterator(*this, m_child.begin());
        }

        virtual const_iterator end() const {
            return const_iterator(*this, m_child.end());
        }
    };

    class NodeVector : public Node {
    protected:
        platformdescription::VecStorage m_child;

    public:
        NodeVector() : Node() {}
        explicit NodeVector(const Origin &o) : Node(o) {}

        virtual NodeType type() const { return PlatformDescription::VECTOR; };

        void push_back(const PlatformDescription &p) { m_child.push_back(p); }

        virtual PlatformDescription& operator[] (const std::string & k) {
            return PlatformDescription::INVALID_DESCRIPTION;
        }

        virtual PlatformDescription& operator[] (size_type i) {
            if (m_child.size() <= i) {
                return PlatformDescription::INVALID_DESCRIPTION;
            }

            return m_child[i];
        }

        virtual size_type size() const { return m_child.size(); }

        virtual iterator begin() {
            return iterator(*this, m_child.begin());
        }

        virtual iterator end() {
            return iterator(*this, m_child.end());
        }

        virtual const_iterator begin() const {
            return const_iterator(*this, m_child.begin());
        }

        virtual const_iterator end() const {
            return const_iterator(*this, m_child.end());
        }
    };

    class NodeScalar : public Node {
    protected:
        std::string m_val;
        eDataTypeHint m_type_hint = STRING;

    public:
        NodeScalar() : Node() {}
        explicit NodeScalar(const Origin &o) : Node(o) {}
        explicit NodeScalar(std::string val) : m_val(val) {}
        NodeScalar(std::string val, const Origin &o) : Node(o), m_val(val) {}
        NodeScalar(std::string val, eDataTypeHint type_hint, const Origin &o)
            : Node(o), m_val(val), m_type_hint(type_hint) {}

        virtual NodeType type() const { return PlatformDescription::SCALAR; };

        virtual PlatformDescription& operator[] (const std::string & k) {
            return PlatformDescription::INVALID_DESCRIPTION;
        }

        virtual PlatformDescription& operator[] (size_type i) {
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

        eDataTypeHint get_type_hint() const { return m_type_hint; }
    };

protected:
    Node *m_node;

    Node * load_yaml_req(YAML::Node root, Node::Origin &origin);
    Node * load_json_req(JSON root, Node::Origin &origin);
    JSON dump_json_req() const;

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
     * @brief Load a JSON description into this description.
     *
     * @param[in] json the JSON description.
     */
    void load_json(const std::string & json);

    /**
     * @brief Dump the PlatformDescription as JSON
     *
     * @return the JSON string representing the PlatformDescription
     */
    std::string dump_json() const;

    /**
     * @brief Load a JSON description from the given file.
     *
     * @param[in] file The path to the file containing the description.
     */
    void load_file_json(const std::string & file);

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
     * @brief Return the size of the root node
     *
     * For map and vector node, the size is the number
     * of elements it contains.
     * For scalar node, the size is always 1.
     * For invalid and nil nodes, the size is always 0.
     *
     * @return the size of the root node.
     */
    size_type size() const { return m_node->size(); }

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
     * @brief Return the child node at index i.
     *
     * If the node type is a vector, this method returns the child node at index i.
     * If the index does not exist, the INVALID_DESCRIPTION is returned.
     *
     * If the node type is not a vector, this method returns the INVALID_DESCRIPTION.
     *
     * @param[in] i the child index.
     *
     * @return the child node.
     */
    PlatformDescription & operator[] (size_type i) { return (*m_node)[i]; }

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

    /**
     * @brief Deep copy this platform description and return the new one.
     *
     * Deep copy this platform description and return the new one.
     *
     * @return the copy of this description.
     */
    PlatformDescription clone() const;

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
    const_iterator begin() const { return static_cast<const Node*>(m_node)->begin() ; }

    /**
     * @brief Return a constant iterator to the <i>past-the-end</i> child node of a map node.
     *
     * @return a constant iterator to the <i>past-the-end</i> child node of a map node.
     */
    const_iterator end() const { return static_cast<const Node*>(m_node)->end(); }

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

        m_node->mark_converted();

        return ret;
    }

    std::string origin() const { return m_node->origin().format(); }

    const Node::Origin & get_origin() const { return m_node->origin(); }

    void visit_non_converted(NodeVisitor &v, std::vector<std::string> &names)
    {
        if (is_map()) {
            for (auto &n : *this) {
                names.push_back(n.first);
                n.second.visit_non_converted(v, names);
                names.pop_back();
            }
        }

        if (is_scalar() && !m_node->has_been_converted()) {
            v(*this, names);
        }
    }

    void visit_non_converted(NodeVisitor &v)
    {
        std::vector<std::string> names;
        visit_non_converted(v, names);
    }

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

template <class T>
inline PlatformDescription::IteratorBase<T>::IteratorBase()
    : m_node(&INVALID_NODE) {}

template <class T>
inline PlatformDescription::IteratorBase<T>::IteratorBase(const Node &node, platformdescription::IteratorState<T> it)
    : m_node(&node), m_it(it) {}

template <class T>
inline PlatformDescription::IteratorBase<T>::IteratorBase(const IteratorBase<T> &it)
    : m_node(it.m_node), m_it(it.m_it) {}

template <class T>
inline PlatformDescription::IteratorBase<T>& PlatformDescription::IteratorBase<T>::operator++()
{
    switch (m_node->type()) {
    case MAP:
        ++m_it.map;
        break;
    case VECTOR:
        ++m_it.vec;
        break;
    case INVALID:
        break;
    default:
        assert(false);
    }

    return *this;
}

template <class T>
inline PlatformDescription::IteratorBase<T>& PlatformDescription::IteratorBase<T>::operator=(const IteratorBase& it)
{
    m_it.map = it.m_it.map;
    m_it.vec = it.m_it.vec;
    m_node = it.m_node;

    return *this;
}

template <class T>
inline bool PlatformDescription::IteratorBase<T>::operator==(const IteratorBase<T>& it)
{
    switch (m_node->type()) {
    case MAP:
        return m_it.map == it.m_it.map;
    case VECTOR:
        return m_it.vec == it.m_it.vec;
    case INVALID:
        return it.m_node->type() == INVALID;
    default:
        assert(false);
        return false;
    }
}

template <class T>
inline bool PlatformDescription::IteratorBase<T>::operator!=(const IteratorBase<T>& it)
{
    switch (m_node->type()) {
    case MAP:
        return m_it.map != it.m_it.map;
    case VECTOR:
        return m_it.vec != it.m_it.vec;
    case INVALID:
        return it.m_node->type() != INVALID;
    default:
        assert(false);
        return false;
    }
}

template <class T>
inline T& PlatformDescription::IteratorBase<T>::operator*() const
{
    platformdescription::IteratorValue val;
    switch (m_node->type()) {
    case MAP:
        return *m_it.map;
    case VECTOR:
        m_pair->second = *m_it.vec;
        return *m_pair;
    case INVALID:
        throw InvalidConversionException("Cannot dereference invalid iterator");
    default:
        assert(false);
        return *m_pair;
    }
}

template <class T>
inline T* PlatformDescription::IteratorBase<T>::operator->() const
{
    platformdescription::IteratorValue val;
    switch (m_node->type()) {
    case MAP:
        return & *m_it.map;
    case VECTOR:
        m_pair->second = *m_it.vec;
        return m_pair.get();
    case INVALID:
        throw InvalidConversionException("Cannot dereference invalid iterator");
    default:
        assert(false);
        return m_pair.get();
    }
}

namespace platformdescription {

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

static inline bool parse_unit(std::stringstream &ss, int64_t &factor)
{
    factor = 1;

    if (!ss.eof()) {
        /* Unit parsing */
        char unit;
        if((!(ss >> unit)) || (!(ss >> std::ws).eof())) {
            return false;
        }

        factor = unit2factor(unit);

        if (!factor) {
            /* Invalid unit */
            return false;
        }
    }

    return true;
}

template <class T>
inline bool parse_digits(const std::string input, T &t)
{
    std::stringstream ss(input);
    int64_t factor;

    ss.unsetf(std::ios::dec);
    if (!(ss >> t)) {
        return false;
    }

    if (!parse_unit(ss, factor)) {
        return false;
    }

    t *= factor;
    return true;
}

/* Specialized bool version. We try to read digits first, and if it fails, we
 * try with boolalpha flag set (false/true instead of 0/1). Boolean do not have units. */
template <>
inline bool parse_digits(const std::string input, bool &t)
{
    std::string lo(input);
    std::transform(lo.begin(), lo.end(), lo.begin(), ::tolower);

    std::stringstream ss(lo);

    if (!(ss >> t)) {
        ss.clear();
        ss.setf(std::ios::boolalpha);
        if (!(ss >> t)) {
            return false;
        }
    }

    if (!(ss >> std::ws).eof()) {
        return false;
    }

    return true;
}

/* Converter for arithmetic types including u?int{8,16,32}_t, float, double and
 * bool. */
template <class T>
struct converter {
    static
    typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
    decode(const PlatformDescription::Node &n, T &res) {
        if (n.type() != PlatformDescription::SCALAR) {
            return false;
        }

        const bool is_neg = n.raw_data().front() == '-';

        if (is_neg && std::is_unsigned<T>::value) {
            return false;
        }

        /*
         * If the target type is floating point, then we use double as the
         * conversion type. If it is an integer, we use int64_t or uint64_t
         * whether it is signed or not.
         */
        typedef typename std::conditional<
            std::is_floating_point<T>::value, double,
            typename std::conditional<
                std::is_signed<T>::value, int64_t, uint64_t>::type >::type _StorageT;

        /* However, if the target type is bool, we use bool */
        typedef typename std::conditional<
            std::is_same<bool, T>::value, bool, _StorageT>::type StorageT;

        StorageT t;

        if (!parse_digits(n.raw_data(), t)) {
            return false;
        }

        if ((t < std::numeric_limits<T>::min())
            || (t > std::numeric_limits<T>::max())) {
            /* input is out of bounds for requested conversion */
            return false;
        }
        res = t;
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

static inline bool str2time_unit(std::string &str, sc_core::sc_time_unit &unit)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

    if (str == "fs") {
        unit = sc_core::SC_FS;
        return true;
    } else if (str == "ps") {
        unit = sc_core::SC_PS;
        return true;
    } else if (str == "ns") {
        unit = sc_core::SC_NS;
        return true;
    } else if (str == "ms") {
        unit = sc_core::SC_MS;
        return true;
    } else if (str == "s") {
        unit = sc_core::SC_SEC;
        return true;
    }

    return false;
}

template <>
struct converter<sc_core::sc_time> {
    static bool decode(const PlatformDescription::Node &n, sc_core::sc_time &res) {
        if (n.type() != PlatformDescription::SCALAR) {
            return false;
        }

        double t;
        sc_core::sc_time_unit unit = sc_core::SC_PS;

        const std::string & input = n.raw_data();
        std::stringstream ss(input);

        ss.unsetf(std::ios::dec);

        if (!(ss >> t)) {
            return false;
        }

        if (!ss.eof()) {
            /* Unit parsing */
            std::string unit_str;
            if((!(ss >> unit_str)) || (!(ss >> std::ws).eof())) {
                return false;
            }

            if (!str2time_unit(unit_str, unit)) {
                return false;
            }
        }
        res = sc_core::sc_time(t, unit);
        return true;
    }
};

template <class T>
struct converter< std::vector<T> > {
private:
    static bool decode_from_vector(const PlatformDescription::Node &n, std::vector<T> &vec)
    {
        for (auto &p : n) {
            try {
                vec.push_back(p.second.as<T>());
            } catch (PlatformDescription::InvalidConversionException e) {
                return false;
            }
        }

        return true;
    }

    static bool decode_from_scalar(const PlatformDescription::Node &n, std::vector<T> &vec) {
        std::list<std::string> toks;
        tokenize(n.raw_data(), ',', toks);

        for (auto &e: toks) {
            T out;
            PlatformDescription::NodeScalar tmp_node(e);

            if (!platformdescription::converter<T>::decode(tmp_node, out)) {
                return false;
            }

            vec.push_back(out);
        }

        return true;
    }

public:
    static bool decode(const PlatformDescription::Node &n, std::vector<T> &vec) {
        switch (n.type()) {
        case PlatformDescription::VECTOR:
            return decode_from_vector(n, vec);
        case PlatformDescription::SCALAR:
            return decode_from_scalar(n, vec);
        default:
            return false;
        }
    }
};

};
#endif
