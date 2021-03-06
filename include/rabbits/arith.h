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

#ifndef _RABBITS_ARITH_H
#define _RABBITS_ARITH_H

/**
 * @file arith.h
 * Arith class declaration
 */

#include <inttypes.h>
#include <set>
#include <vector>
#include <iterator>

#include "rabbits/datatypes/address_range.h"

/**
 * @brief Arithmetic and logic helpers class.
 */
class Arith {
public:
    /**
     * @brief Find the last set bit in a 32-bit word.
     *
     * Find the last set bit in a 32-bit word.
     * The result is undefined if w equals 0.
     *
     * @param[in] w the 32-bit word
     *
     * @return The index of the last set bit in w.
     */
    static int fls32(uint32_t w) {
        return (sizeof(w) << 3) - __builtin_clz(w) - 1;
    }

    /**
     * @brief Return true if w is a power of 2
     */
    static bool is_power_of_2(uint64_t w) {
        if (!w) {
            return false;
        }

        return !(w & (w-1));
    }

private:
    class AddressRangeOrder {
    public:
        bool operator() (const AddressRange &a, const AddressRange &b) const {
            return a.begin() < b.begin();
        }
    };

public:
    static void neg_memmap32(const std::vector<AddressRange> &map,
                             std::vector<AddressRange> &out)
    {
        std::set<AddressRange, AddressRangeOrder> in;
        uint64_t cur = 0;

        std::copy(map.begin(), map.end(), std::inserter(in, in.begin()));

        for (const AddressRange &r : in) {
            if (r.begin() > cur) {
                out.push_back(AddressRange(cur, r.begin() - cur));
            }
            cur = r.end() + 1;
        }

        if (cur < 0xffffffff) {
            out.push_back(AddressRange(cur, 0xffffffff - cur + 1));
        }
    }
};
#endif
