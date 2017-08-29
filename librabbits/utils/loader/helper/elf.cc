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

#include <elf.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "rabbits/logger.h"
#include "rabbits/component/debug_initiator.h"
#include "elf.h"

static void resize_buffer(std::vector<uint8_t>& buf, uint64_t size)
{
    if (buf.capacity() < size) {
        buf.reserve(size);
    }
}

template <class T_hdr, class T_phdr>
static int load_elf(int fd, DebugInitiator &bus, uint64_t *entry)
{
    T_hdr hdr;
    T_phdr *phdr = NULL;
    ssize_t phdr_size;
    int i;
    int64_t written;
    std::vector<uint8_t> buf;

    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        goto fail;
    }

    lseek(fd, hdr.e_phoff, SEEK_SET);
    phdr = new T_phdr[hdr.e_phnum];

    phdr_size = sizeof(phdr[0]) * hdr.e_phnum;
    if (read(fd, phdr, phdr_size) != phdr_size) {
        goto fail;
    }

    if (entry) {
        *entry = static_cast<uint64_t>(hdr.e_entry);
    }

    LOG_F(APP, DBG, "Loading elf with %d sections\n", hdr.e_phnum);

    for (i = 0; i < hdr.e_phnum; i++) {
        T_phdr *ph = &phdr[i];

        if (ph->p_type == PT_LOAD) {
            LOG_F(APP, DBG, "Loading elf segment, start:%08" PRIx64
                  ", size:%08" PRIx64 "\n",
                  static_cast<uint64_t>(ph->p_paddr),
                  static_cast<uint64_t>(ph->p_filesz));

            const uint64_t offset = ph->p_offset;
            const int64_t filesize = ph->p_filesz;

            resize_buffer(buf, filesize);

            if (lseek(fd, offset, SEEK_SET) < 0) {
                perror("lseek");
                goto fail;
            }

            if (read(fd, &(buf[0]), filesize) != filesize) {
                LOG(APP, ERR) << "Error while reading elf file\n";
                goto fail;
            }

            written = bus.debug_write(ph->p_paddr, &(buf[0]), filesize);

            if (written < filesize) {
                LOG_F(APP, ERR, "Only %" PRIu64 " bytes were written "
                      "over %" PRIu64 ". "
                      "Trying to write outside ram?\n",
                      written, static_cast<uint64_t>(filesize));
                goto fail;
            }
        }
    }

    delete [] phdr;
    return 0;

fail:
    delete [] phdr;
    return 1;
}

/**
 * @brief Load an ELF image to platform memory.
 *
 * This method uses a DebugInitiator to write to the platform memory.
 * It can return the ELF entry point into the entry pointer if it is not NULL.
 *
 * @param[in] elf_fn Path to the ELF image to load.
 * @param[in,out] bus The DebugInitiator used to write to memory.
 * @param[out] entry The ELF entry point.
 *
 * @return 0 on success, a positive value on error.
 */
static int load_elf(const std::string & elf_fn, DebugInitiator &bus, uint64_t *entry)
{
    int fd, ret = 0;
    uint8_t e_ident[EI_NIDENT];

    fd = open(elf_fn.c_str(), O_RDONLY);
    if (fd < 0) {
        perror("open");
        ret = 1;
        goto open_fail;
    }

    if (read(fd, e_ident, sizeof(e_ident)) != sizeof(e_ident)) {
        ret = 1;
        goto fail;
    }

    if (e_ident[0] != ELFMAG0 ||
        e_ident[1] != ELFMAG1 ||
        e_ident[2] != ELFMAG2 ||
        e_ident[3] != ELFMAG3) {
        ret = 2;
        goto fail;
    }

    lseek(fd, 0, SEEK_SET);

    if (e_ident[EI_CLASS] == ELFCLASS64) {
        ret = load_elf<Elf64_Ehdr, Elf64_Phdr>(fd, bus, entry);
    } else {
        ret = load_elf<Elf32_Ehdr, Elf32_Phdr>(fd, bus, entry);
    }

fail:
    close(fd);
open_fail:
    return ret;
}

void ElfLoaderHelper::load_file(const std::string &fn, DebugInitiator &di,
                                uint64_t load_addr, ImageLoadResult &result)
{
    int ret = load_elf(fn, di, &result.entry_point);

    switch (ret) {
    case 0:
        result.result = ImageLoadResult::LOAD_SUCCESS;
        result.has_entry_point = true;
        break;
    case 2:
        result.result = ImageLoadResult::INCOMPATIBLE;
        break;
    default:
        result.result = ImageLoadResult::LOAD_ERROR;
    }
}

void ElfLoaderHelper::load_data(const void *data, size_t len, DebugInitiator &di,
                                uint64_t load_addr, ImageLoadResult &result)
{
    /* TODO */
    result.result = ImageLoadResult::INCOMPATIBLE;
}

