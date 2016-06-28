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

#include "rabbits/logger.h"
#include "rabbits/platform/description.h"

#include <yaml-cpp/yaml.h>
#include <sstream>

using std::string;
using std::list;
using std::map;
using std::vector;
using std::set;

/*
 * ====================
 * PlatformDescription
 * ====================
 */
PlatformDescription::NodeInvalid
PlatformDescription::INVALID_NODE(true);

PlatformDescription
PlatformDescription::INVALID_DESCRIPTION(&PlatformDescription::INVALID_NODE);

PlatformDescription::PlatformDescription()
{
    m_node = new NodeNil;
    m_node->inc_ref();
}

PlatformDescription::PlatformDescription(Node *n)
{
    assert(n);
    m_node = n;
    m_node->inc_ref();
}

PlatformDescription::PlatformDescription(const PlatformDescription &p)
{
    m_node = p.m_node;
    m_node->inc_ref();
}

PlatformDescription::~PlatformDescription()
{
    m_node->dec_ref();

    if (m_node->is_deletable()) {
        delete m_node;
    }
}

PlatformDescription::Node *
PlatformDescription::load_yaml_req(YAML::Node root, Node::Origin &origin)
{
    YAML::Node::iterator it;
    Node *n = NULL;
    NodeMap *nm = NULL;
    NodeVector *nv = NULL;

    origin.line = root.Mark().line;
    origin.column = root.Mark().column;

    switch (root.Type()) {
    case YAML::NodeType::Map:
        nm = new NodeMap(origin);
        for (it = root.begin(); it != root.end(); it++) {
            (*nm)[it->first.as<string>()] =
                PlatformDescription(load_yaml_req(it->second, origin));
        }
        n = nm;
        break;

    case YAML::NodeType::Sequence:
        nv = new NodeVector(origin);
        for (it = root.begin(); it != root.end(); it++) {
            nv->push_back(PlatformDescription(load_yaml_req(*it, origin)));
        }
        n = nv;
        break;

    case YAML::NodeType::Scalar:
        n = new NodeScalar(root.as<string>(), origin);
        break;

    case YAML::NodeType::Null:
        n = new NodeNil(origin);
        break;

    default:
        assert(false);
        break;
    }

    return n;
}

void PlatformDescription::load_file_yaml(const string &file)
{
    YAML::Node y_root = YAML::LoadFile(file);
    Node::Origin origin(file, 0, 0);

    *this = PlatformDescription(load_yaml_req(y_root, origin));
}

void PlatformDescription::load_yaml(const string &yaml)
{
    YAML::Node y_root = YAML::Load(yaml);
    Node::Origin origin;
    *this = PlatformDescription(load_yaml_req(y_root, origin));
}

void PlatformDescription::tokenize_arg(const string arg, list<string>& toks)
{
    std::istringstream ss(arg);
    string tok;

    while(std::getline(ss, tok, '.')) {
        toks.push_back(tok);
    }
}

PlatformDescription::NodeScalar*
PlatformDescription::parse_arg_req(list<string>& toks, Node::Origin &origin)
{
    Node::Origin cur_orig = origin;

    if (toks.empty()) {
        if ((type() != NIL) && (type() != SCALAR)) {
            throw InvalidCmdLineException("");
            abort();
        }

        NodeScalar *n = new NodeScalar("", origin);
        *this = PlatformDescription(n);
        return n;
    }

    string tok = toks.front();
    toks.pop_front();

    switch (type()) {
    case NIL:
        *this = PlatformDescription(new NodeMap(cur_orig));
        /* fallthrough */
    case MAP:
        origin.column += tok.length() + 1;
        return (*m_node)[tok].parse_arg_req(toks, origin);
        break;

    case VECTOR:
    case SCALAR:
    case INVALID:
        throw InvalidCmdLineException("");
        break;
    }

    return NULL;
}

void PlatformDescription::parse_cmdline(int argc, const char * const argv[],
                                        const set<string> &unaries)
{
    enum { ARG, VAL } state = ARG;
    list<string> toks;
    NodeScalar *n;
    Node::Origin origin(Node::Origin::CMDLINE);

    origin.column += string(argv[0]).length();

    for(int i = 1; i < argc; i++) {
        string arg(argv[i]);

        switch (state) {
        case ARG:
            {
                if (arg[0] != '-') {
                    throw InvalidCmdLineException(arg);
                }

                const string sarg = arg.substr(1);

                tokenize_arg(sarg, toks);

                try {
                    n = parse_arg_req(toks, origin);
                } catch (InvalidCmdLineException) {
                    throw InvalidCmdLineException(arg);
                }

                if (unaries.find(sarg) != unaries.end()) {
                    /* Unary argument, no value. Set it to true and carry on */
                    n->set_raw_data("true");
                    state = ARG;
                } else {
                    state = VAL;
                }
            }
            break;

        case VAL:
            n->set_raw_data(arg);
            state = ARG;
            origin.column += arg.length();
            break;
        }
    }
}

PlatformDescription& PlatformDescription::operator=(const PlatformDescription& p)
{
    m_node->dec_ref();
    if (m_node->is_deletable()) {
        delete m_node;
    }

    m_node = p.m_node;
    m_node->inc_ref();

    return *this;
}

void PlatformDescription::alias(const string& root, const string& child)
{
    /* XXX */
}

#ifdef DEBUG_MERGE
# define DBG_MERGE DBG_STREAM
#else
# define DBG_MERGE(...) do {} while(0)
#endif

PlatformDescription PlatformDescription::merge(PlatformDescription &p)
{
    DBG_MERGE("--> Start merge\n");

    switch(type()) {
    case NIL:
        DBG_MERGE("<-- this is nil, returning p\n");
        return p;
        break;

    case MAP:
        if (p.is_map()) {
            DBG_MERGE("this and p are map\n");

            NodeMap *n = new NodeMap;

            iterator it;
            set<string> to_merge;

            for (it = p.begin(); it != p.end(); it++) {
                to_merge.insert(it->first);
            }

            for (it = begin(); it != end(); it++) {
                const string key = it->first;

                if (p.exists(key)) {
                    DBG_MERGE("same key " << key << " on both side, we must go deeper\n");
                    (*n)[key] = it->second.merge(p[key]);
                    to_merge.erase(key);
                } else {
                    assert(to_merge.find(key) == to_merge.end());
                    DBG_MERGE("key " << key << " only on this side, merging\n");
                    (*n)[key] = it->second;
                }
            }

            for (set<string>::iterator it2 = to_merge.begin(); it2 != to_merge.end(); it2++) {
                DBG_MERGE("key " << *it2 << " only on p side, merging\n");
                (*n)[*it2] = p[*it2];
            }

            DBG_MERGE("<-- returning merged node\n");
            return PlatformDescription(n);

        } else {
            DBG_MERGE("<-- this is map, returning this\n");
            return *this;
        }
        break;

    case SCALAR:
        DBG_MERGE("<-- this is scalar, returning this\n");
        return *this;
        break;

    default:
        assert(false);
        break;
    }
}

static void indent(std::ostream &os, int lvl)
{
    for (int i = 0; i < lvl; i++) {
        os << "  ";
    }
}

void PlatformDescription::dump(std::ostream &os, int lvl) const
{
    switch(type()) {
    case NIL:
        indent(os, lvl);
        os << "(nil)\n";
        break;

    case SCALAR:
        indent(os, lvl);
        os << m_node->raw_data() << "\n";
        break;

    case MAP:
        for (auto e : *this) {
            indent(os, lvl);
            os << e.first << ":" << "\n";
            e.second.dump(os, lvl+1);
        }
        break;

    default:
        assert(false);
        break;
    }
}

void PlatformDescription::dump(std::ostream &os) const
{
    dump(os, 0);
}

