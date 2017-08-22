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

#pragma once

#include <cstdint>
#include <cstring>
#include <cassert>
#include <ostream>

class PixelInfo {
public:
    enum PixelOrdering
    {
        RGBA = 0,
        BGRA,
        ARGB,
        ABGR,
        INDEXED,
    };

    constexpr static uint32_t get_pixel_format_id(PixelOrdering f, int bpp,
                                                  int r, int g, int b, int a)
    {
        return (f << 24) | (bpp << 16) | (r << 12) | (g << 8) | (b << 4) | a;
    }

    struct ComponentInfo {
        int idx;
        int pos;
        int size;
        char sym;

        ComponentInfo(char sym) : sym(sym) {}
    };

private:
    uint32_t m_id;
    ComponentInfo m_r { 'R' }, m_g { 'G' }, m_b { 'B' }, m_a { 'A' };
    ComponentInfo *m_ordered[4] { nullptr };

    void set_ordered()
    {
        if (is_indexed()) {
            return;
        }

        m_ordered[m_r.idx] = &m_r;
        m_ordered[m_g.idx] = &m_g;
        m_ordered[m_b.idx] = &m_b;
        m_ordered[m_a.idx] = &m_a;
    }

    void set_components()
    {
        switch(get_ordering()) {
        case RGBA:
            m_r.idx = 0;
            m_g.idx = 1;
            m_b.idx = 2;
            m_a.idx = 3;
            break;

        case BGRA:
            m_b.idx = 0;
            m_g.idx = 1;
            m_r.idx = 2;
            m_a.idx = 3;
            break;

        case ARGB:
            m_a.idx = 0;
            m_r.idx = 1;
            m_g.idx = 2;
            m_b.idx = 3;
            break;

        case ABGR:
            m_a.idx = 0;
            m_b.idx = 1;
            m_g.idx = 2;
            m_r.idx = 3;
            break;

        case INDEXED:
            return;
        }

        set_ordered();

        int pos = 0;
        for (int i = 0; i < 4; i++) {
            m_ordered[i]->pos = pos;
            pos += m_ordered[i]->size;
        }
    }

public:
    PixelInfo(uint32_t pixel_fmt_id) : m_id(pixel_fmt_id)
    {
        m_r.size = (m_id >> 12) & 0xf;
        m_g.size = (m_id >> 8) & 0xf;
        m_b.size = (m_id >> 4) & 0xf;
        m_a.size = m_id & 0xf;
        set_components();
    }

    /* Default to ARGB_8888 */
    PixelInfo() : PixelInfo(get_pixel_format_id(ARGB, 32, 8, 8, 8, 8)) {}

    PixelInfo(const PixelInfo &p)
    {
        std::memcpy(this, &p, sizeof(PixelInfo));
        set_ordered();
    }

    PixelInfo & operator=(const PixelInfo &p)
    {
        std::memcpy(this, &p, sizeof(PixelInfo));
        set_ordered();

        return *this;
    }

    PixelOrdering get_ordering() const
    {
        return PixelOrdering(m_id >> 24);
    }

    bool is_indexed() const
    {
        return get_ordering() == INDEXED;
    }

    uint32_t get_id() const { return m_id; }
    int get_bpp() const { return (m_id >> 16) & 0xff; }

    ComponentInfo get_r() const { return m_r; }
    ComponentInfo get_g() const { return m_g; }
    ComponentInfo get_b() const { return m_b; }
    ComponentInfo get_a() const { return m_a; }

    ComponentInfo get_comp(int idx) const { assert(idx < 4); return *m_ordered[idx]; }
};

static inline std::ostream & operator << (std::ostream & s, const PixelInfo &p)
{
    s << p.get_bpp() << "bpp ";

    if (p.is_indexed()) {
        s << "indexed";
        return s;
    }

    for (int i = 0; i < 4; i++) {
        const PixelInfo::ComponentInfo c = p.get_comp(i);

        if (!c.size) {
            continue;
        }

        s << c.sym << c.size;
    }

    return s;
}


