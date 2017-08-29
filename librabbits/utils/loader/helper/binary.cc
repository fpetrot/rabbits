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

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "rabbits/logger.h"
#include "rabbits/component/debug_initiator.h"

#include "binary.h"

void BinaryLoaderHelper::load_file(const std::string &fn, DebugInitiator &di,
                                   uint64_t load_addr, ImageLoadResult &result)
{
    int fd;
    off_t fsize;
    void *data = NULL;

    LOG_F(APP, DBG, "Loading image %s at 0x%" PRIx64 "\n", fn.c_str(), load_addr);

    fd = open(fn.c_str(), O_RDONLY);
    if (fd < 0) {
        perror("open");
        result.result = ImageLoadResult::LOAD_ERROR;
        goto open_fail;
    }

    fsize = lseek(fd, 0, SEEK_END);
    if (fsize == -1) {
        perror("lseek");
        result.result = ImageLoadResult::LOAD_ERROR;
        goto lseek_fail;
    }

    data = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        result.result = ImageLoadResult::LOAD_ERROR;
        goto mmap_fail;
    }

    load_data(data, fsize, di, load_addr, result);

    //written = di.debug_write(load_addr, data, fsize);
    //if(written < static_cast<uint64_t>(fsize)) {
        //LOG_F(APP, ERR, "Only %" PRIu64 " bytes were written over %" PRIu64
              //". Trying to write outside ram?\n",
              //written, static_cast<uint64_t>(fsize));
        //result.result = ImageLoadResult::LOAD_ERROR;
        //goto write_fail;
    //}

    munmap(data, fsize);
    data = NULL;
mmap_fail:
lseek_fail:
    close(fd);
open_fail:
    return;
}

void BinaryLoaderHelper::load_data(const void *data, size_t len, DebugInitiator &di,
                                   uint64_t load_addr, ImageLoadResult &result)
{
    uint64_t written;

    LOG_F(APP, DBG, "Loading data (%d bytes) at 0x%" PRIx64 "\n", len, load_addr);

    result.has_entry_point = false;
    result.has_load_size = true;
    result.load_size = len;

    written = di.debug_write(load_addr, data, len);
    if(written < len) {
        LOG_F(APP, ERR, "Only %" PRIu64 " bytes were written over %" PRIu64
              ". Trying to write outside ram?\n", written, len);
        result.result = ImageLoadResult::LOAD_ERROR;
        return;
    }

    result.result = ImageLoadResult::LOAD_SUCCESS;
}
