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

#include "pixel.h"

class PixelConverter {
public:
    class BitsIStream {
    private:
        const uint8_t *m_buf;
        uint8_t m_cur;
        int m_left;

    public:
        BitsIStream(const uint8_t *buffer) : m_buf(buffer)
        {
            m_cur = *m_buf++;
            m_left = 8;
        }

        uint8_t get_bits(int count)
        {
            uint8_t ret = 0;
            int sh = 0;

            assert(count <= 8);

            if (count > m_left) {
                ret = m_cur;
                m_cur = *(m_buf++);
                sh = m_left;
                count -= m_left;
                m_left = 8;
            }

            ret |= ((m_cur & ((1 << count) - 1)) << sh);
            m_left -= count;
            m_cur >>= count;

            return ret;
        }

    };

    class BitsOStream {
    private:
        uint8_t *m_buf;
        int m_left;

    public:
        BitsOStream(uint8_t *buffer) : m_buf(buffer)
        {
            m_left = 8;
        }

        void put_bits(uint8_t data, int count)
        {
            assert(count <= 8);

            if (count > m_left) {
                *m_buf = (*m_buf & ((1 << (8 - m_left)) - 1))
                    | ((data & ((1 << m_left) - 1)) << (8 - m_left));

                data >>= m_left;
                count -= m_left;

                m_left = 8;
                m_buf++;
            }

            *m_buf = (*m_buf & ((1 << (8 - m_left)) - 1))
                | ((data & ((1 << count) - 1)) << (8 - m_left));

            m_left -= count;

            if (!m_left) {
                m_left = 8;
                m_buf++;
            }
        }

    };


private:
    PixelInfo m_src, m_dst;
    int m_idx_map[4];

    uint8_t normalize(uint32_t data, int src_size, int dst_size)
    {
        if (src_size == dst_size) {
            return data;
        }

        return data * ((1 << dst_size) - 1) / ((1 << src_size) - 1);
    }

public:
    PixelConverter(const PixelInfo &src_fmt, const PixelInfo &dst_fmt)
        : m_src(src_fmt), m_dst(dst_fmt)
    {
        m_idx_map[src_fmt.get_r().idx] = dst_fmt.get_r().idx;
        m_idx_map[src_fmt.get_g().idx] = dst_fmt.get_g().idx;
        m_idx_map[src_fmt.get_b().idx] = dst_fmt.get_b().idx;
        m_idx_map[src_fmt.get_a().idx] = dst_fmt.get_a().idx;
    }

    void convert(const uint8_t *src, uint8_t *dst, size_t count)
    {
        BitsIStream in(src);
        BitsOStream out(dst);

        while(count--) {
            uint32_t cs[4];

            for (int i = 0; i < 4; i++) {
                const PixelInfo::ComponentInfo &src_comp = m_src.get_comp(i);
                const PixelInfo::ComponentInfo &dst_comp = m_dst.get_comp(m_idx_map[i]);

                assert(src_comp.sym == dst_comp.sym);

                if (!src_comp.size) {
                    cs[m_idx_map[i]] = 0;
                    continue;
                }

                uint32_t c = in.get_bits(src_comp.size);

                if (!dst_comp.size) {
                    continue;
                }

                cs[m_idx_map[i]] = normalize(c, src_comp.size, dst_comp.size);
            }

            for (int i = 0; i < 4; i++) {
                const PixelInfo::ComponentInfo &dst_comp = m_dst.get_comp(i);

                if (!dst_comp.size) {
                    continue;
                }

                out.put_bits(cs[i], dst_comp.size);
            }
        }
    }
};

