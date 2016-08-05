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

#include "rabbits/config.h"
#include "rabbits/rabbits_exception.h"

#if defined(RABBITS_DEBUG) && defined(RABBITS_CONFIG_POSIX)
    /* XXX GNU extension */
    #define GNU_BACKTRACE
    #include <execinfo.h>
    #include <cstdlib>
    #include <sstream>
    #include <cxxabi.h>
    #include <iomanip>
    #include <inttypes.h>
#endif

using std::string;
using std::stringstream;

#ifdef GNU_BACKTRACE
static void split_trace(string trace, string &obj, string &symbol, uint64_t &offset, uint64_t &address)
{
    size_t pos, last;


    pos = trace.find_first_of('(');
    obj = trace.substr(0, pos);
    last = pos + 1;

    pos = trace.find_first_of('+', last);
    symbol = trace.substr(last, pos - last);
    last = pos + 1;

    {
        stringstream conv;
        pos = trace.find_first_of(')', last);
        conv.str(trace.substr(last, pos - last));
        conv.unsetf(std::ios::dec);
        conv >> offset;
        last = pos + 3;
    }

    {
        stringstream conv;
        pos = trace.find_first_of(']', last);
        conv.str(trace.substr(last, pos - last));
        conv.unsetf(std::ios::dec);
        conv >> address;
        last = pos + 1;
    }
}
#endif

void RabbitsException::make_backtrace() {
#ifdef GNU_BACKTRACE
        const int SIZE = 100;
        void *buf[SIZE];
        char **strings;
        char *demangled = NULL;
        size_t dem_size = 0;
        stringstream bt;
        char fill;

        int nptrs = backtrace(buf, SIZE);
        strings = backtrace_symbols(buf, nptrs);

        if (strings == NULL) {
            m_backtrace = "<backtrace error>";
            return;
        }
        
        for (int i = 0; i < nptrs; i++) {
            int status;
            string obj, symbol;
            uint64_t offset, address;
            std::ios::fmtflags flags;

            split_trace(strings[i], obj, symbol, offset, address);

            demangled = abi::__cxa_demangle(symbol.c_str(), demangled, &dem_size, &status);
            if (!status) {
                symbol = demangled;
            }

            flags = bt.flags();
            fill = bt.fill();
            bt << '#' << std::setw(2) << std::left << i 
               << " 0x" << std::hex << std::setw(16) << std::setfill('0') << std::right << address 
               << " in " << symbol << " from " << obj << "\n";
            bt.fill(fill);
            bt.flags(flags);
        }

        free(strings);
        free(demangled);

        m_backtrace = bt.str();
#else
        m_backtrace = "";
#endif
}
