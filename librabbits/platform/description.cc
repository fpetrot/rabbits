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
PlatformDescription::load_yaml_req(YAML::Node root)
{
    YAML::Node::iterator it;
    Node *n = NULL;
    NodeMap *nm = NULL;
    NodeVector *nv = NULL;

    switch (root.Type()) {
    case YAML::NodeType::Map:
        nm = new NodeMap;
        for (it = root.begin(); it != root.end(); it++) {
            (*nm)[it->first.as<string>()] =
                PlatformDescription(load_yaml_req(it->second));
        }
        n = nm;
        break;

    case YAML::NodeType::Sequence:
        nv = new NodeVector;
        for (it = root.begin(); it != root.end(); it++) {
            nv->push_back(PlatformDescription(load_yaml_req(*it)));
        }
        n = nv;
        break;

    case YAML::NodeType::Scalar:
        n = new NodeScalar(root.as<string>());
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
    *this = PlatformDescription(load_yaml_req(y_root));
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
PlatformDescription::parse_arg_req(list<string>& toks)
{
    if (toks.empty()) {
        if ((type() != NIL) && (type() != SCALAR)) {
            throw InvalidCmdLineException("");
            abort();
        }

        NodeScalar *n = new NodeScalar("");
        *this = PlatformDescription(n);
        return n;
    }

    string tok = toks.front();
    toks.pop_front();

    switch (type()) {
    case NIL:
        *this = PlatformDescription(new NodeMap);
        /* fallthrough */
    case MAP:
        return (*m_node)[tok].parse_arg_req(toks);
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
                    n = parse_arg_req(toks);
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

PlatformDescription PlatformDescription::merge(PlatformDescription &p)
{
    switch(type()) {
    case NIL:
        return p;
        break;

    case MAP:
        if (p.is_map()) {
            NodeMap *n = new NodeMap;

            iterator it;
            set<string> to_merge;

            for (it = p.begin(); it != p.end(); it++) {
                to_merge.insert(it->first);
            }

            for (it = begin(); it != end(); it++) {
                const string key = it->first;

                if (p.exists(key)) {
                    (*n)[key] = it->second.merge(p[key]);
                    to_merge.erase(key);
                } else {
                    (*n)[key] = it->second;
                }
            }

            for (set<string>::iterator it2 = to_merge.begin(); it2 != to_merge.end(); it2++) {
                (*n)[*it2] = p[*it2];
            }

            return PlatformDescription(n);

        } else {
            return *this;
        }
        break;

    case SCALAR:
        return *this;
        break;

    default:
        assert(false);
        break;
    }
}
