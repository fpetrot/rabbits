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
 * @file loader.h
 * @brief Loader class declaration
 */

#ifndef _UTILS_LOADER_H
#define _UTILS_LOADER_H

#include <string>
#include <elf.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "rabbits/component/debug_initiator.h"

#include "rabbits/logger.h"

/**
 * @brief Helper class to load binary and ELF file to platform memory.
 */
class Loader {
private:
    /* Align addr on page boundary, as required by mmap
     *  Return the offset to apply to modified addr to obtain original addr
     */
    static uint64_t align_addr_on_page(uint64_t &addr)
    {
        long ps = sysconf(_SC_PAGESIZE);
        uint64_t offset;

        LOG_F(APP, DBG, "align: %08" PRIx64 "\n", addr);
        offset = addr & (ps - 1);
        addr &= ~(ps - 1);

        LOG_F(APP, DBG, "aligned: %08" PRIx64 ", off:%08" PRIx64 "\n", addr, offset);

        return offset;
    }

    template <class T_hdr, class T_phdr>
    static int load_elf(int fd, DebugInitiator *bus, uint64_t *entry)
    {
        T_hdr hdr;
        T_phdr *phdr = NULL;
        ssize_t phdr_size;
        int i;
        void *elf_data = NULL;
        uint64_t written;

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
                LOG_F(APP, DBG, "Loading elf segment, start:%08" PRIx64 ", size:%08" PRIx64 "\n",
		      static_cast<uint64_t>(ph->p_paddr),
		      static_cast<uint64_t>(ph->p_filesz));

                uint64_t offset = ph->p_offset;
                uint64_t rem;

                rem = align_addr_on_page(offset);

                elf_data = mmap(NULL, ph->p_filesz, PROT_READ, MAP_SHARED, fd, offset);

                if (elf_data == MAP_FAILED) {
                    perror("mmap");
                    goto fail;
                }

                written = bus->debug_write(ph->p_paddr, ((char*)elf_data)+rem, ph->p_filesz);
                if (written < ph->p_filesz) {
                    LOG_F(APP, ERR, "Only %" PRIu64 " bytes were written over %" PRIu64 ". Trying to write outside ram?\n", 
			  written, static_cast<uint64_t>(ph->p_filesz));
                    munmap(elf_data, ph->p_filesz);
                    elf_data = NULL;
                    goto fail;
                }

                munmap(elf_data, ph->p_filesz);
                elf_data = NULL;
            }
        }

        delete [] phdr;
        return 0;

fail:
        delete [] phdr;
        return 1;
    }

public:
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
    static int load_elf(const std::string & elf_fn, DebugInitiator *bus, uint64_t *entry)
    {
        int fd, ret;
        uint8_t e_ident[EI_NIDENT];

        fd = open(elf_fn.c_str(), O_RDONLY);
        if (fd < 0) {
            perror("read");
            goto open_fail;
        }

        if (read(fd, e_ident, sizeof(e_ident)) != sizeof(e_ident)) {
            goto fail;
        }

        if (e_ident[0] != ELFMAG0 ||
            e_ident[1] != ELFMAG1 ||
            e_ident[2] != ELFMAG2 ||
            e_ident[3] != ELFMAG3) {
            goto fail;
        }

        lseek(fd, 0, SEEK_SET);

        if (e_ident[EI_CLASS] == ELFCLASS64) {
            ret = load_elf<Elf64_Ehdr, Elf64_Phdr>(fd, bus, entry);
        } else {
            ret = load_elf<Elf32_Ehdr, Elf32_Phdr>(fd, bus, entry);
        }

        close(fd);

        return ret;

fail:
        close(fd);
open_fail:
        return 1;
    }

    /**
     * @brief Load a binary image to platform memory.
     *
     * This method uses a DebugInitiator to write to the platform memory.
     * It can return the binary image size into the img_size pointer if it is not NULL.
     *
     * @param[in] img_fn Path to the binary image.
     * @param[in,out] bus The DebugInitiator used to write to memory.
     * @param[in] load_addr Load address of the binary image.
     * @param[out] img_size The binary image size.
     *
     * @return 0 on success, a positive value on error.
     */
    static int load_image(const std::string & img_fn, DebugInitiator *bus, uint64_t load_addr, uint64_t *img_size)
    {
        int fd;
        off_t fsize;
        void *data = NULL;
        uint64_t written;

        LOG_F(APP, DBG, "Loading image %s at 0x%" PRIx64 "\n", img_fn.c_str(), load_addr);

        fd = open(img_fn.c_str(), O_RDONLY);
        if (fd < 0) {
            perror("read");
            goto open_fail;
        }

        fsize = lseek(fd, 0, SEEK_END);
        if (fsize == -1) {
            perror("lseek");
            goto fail;
        }

        if (img_size) {
            *img_size = fsize;
        }

        data = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED) {
            perror("mmap");
            goto fail;
        }

        written = bus->debug_write(load_addr, data, fsize);
        if(written < static_cast<uint64_t>(fsize)) {
            LOG_F(APP, ERR, "Only %" PRIu64 " bytes were written over %" PRIu64 ". Trying to write outside ram?\n", 
		  written, static_cast<uint64_t>(fsize));
            goto fail;
        }

        munmap(data, fsize);
        data = NULL;

        return 0;

fail:
        if (data) {
            munmap(data, fsize);
        }

        close(fd);
open_fail:
        return 1;
    }
};

#endif
