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

#ifndef _RABBITS_DATATYPES_INDEXED_POOL_H
#define _RABBITS_DATATYPES_INDEXED_POOL_H

#include "rabbits/rabbits_exception.h"

/**
 * @brief An Indexed pool.
 *
 * @tparam KEY the type used for the primary index
 * @tparam T the type used in the pool.
 */
template <typename KEY, typename T>
class IndexedPool {
public:
    class IndexNotFoundException : public RabbitsException {
    public:
        IndexNotFoundException() : RabbitsException("Index not found.") {}
        virtual ~IndexNotFoundException() throw() {}
    };

protected:
    std::map<KEY, T> m_elts;
    std::vector<T> m_by_idx;

public:
    IndexedPool() {}
    virtual ~IndexedPool() {}

    void add(const KEY &name, const T &elt) {
        m_elts[name] = elt;
        m_by_idx.push_back(m_elts[name]);
    }

    bool exists(const KEY &id) const {
        return m_elts.find(id) != m_elts.end();
    }

    bool exists(unsigned int idx) const {
        return m_by_idx.size() > idx;
    }

    T& operator[] (const KEY id) {
        if (!exists(id)) {
            throw IndexNotFoundException();
        }
        return m_elts[id];
    }

    T& operator[] (unsigned int idx) {
        if (!exists(idx)) {
            throw IndexNotFoundException();
        }
        return m_by_idx[idx];
    }

    typedef typename std::map<KEY, T>::iterator iterator;
    typedef typename std::map<KEY, T>::const_iterator const_iterator;

    iterator begin() { return m_elts.begin(); }
    iterator end() { return m_elts.end(); }
    const_iterator begin() const { return m_elts.begin(); }
    const_iterator end() const { return m_elts.end(); }
};

#endif
