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

#ifndef _UTILS_BOOTLOADER_H
#define _UTILS_BOOTLOADER_H

#include "rabbits/component/debug_initiator.h"

#include <vector>

class ArmBootloader
{
public:
    /* Relative to start of ram */
    static const uint32_t DTB_DEFAULT_LOAD_ADDR = 128 * 1024 * 1024;
    static const uint32_t KERNEL_DEFAULT_LOAD_ADDR = 32 * 1024 * 1024;

    enum fixup_e {
        FIXUP_NONE = 0,
        FIXUP_MACHINE_ID,
        FIXUP_BOOT_DATA,
        FIXUP_KERNEL_ENTRY,
        FIXUP_SMP_BOOTREG,
        FIXUP_SECONDARY_ENTRY,

        NUM_FIXUP
    };


    class PatchBlob
    {
    public:
        struct Entry
        {
            uint32_t insn;
            fixup_e  fixup;
        };

    protected:
        std::vector<Entry> m_blob;

    public:
        PatchBlob() {}
        PatchBlob(const Entry blob[], size_t size);

        void patch(const uint32_t patch_ctx[NUM_FIXUP]);

        bool empty() { return m_blob.empty(); }
        uint32_t size() { return m_blob.size() * 4; }
        int load(uint32_t addr, DebugInitiator *bus);
    };

protected:
    PatchBlob m_entry;
    PatchBlob m_secondary_entry;
    DebugInitiator *m_bus;

    uint32_t m_machine_id;
    uint32_t m_ram_start;

    std::string m_kernel_path, m_initramfs_path, m_dtb_path;
    uint32_t m_kernel_load_addr, m_initramfs_load_addr, m_dtb_load_addr;

public:
    ArmBootloader(DebugInitiator *bus);
    virtual ~ArmBootloader();

    int load_image(const std::string & path, uint64_t load_addr);

    void set_kernel_image(const std::string & path) { m_kernel_path = path; }
    void set_kernel_load_addr(uint32_t addr) { m_kernel_load_addr = addr; }

    void set_initramfs_image(const std::string & path) { m_initramfs_path = path; }
    void set_initramfs_load_addr(uint32_t addr) { m_initramfs_load_addr = addr; }

    void set_dtb(const std::string & path) { m_dtb_path = path; }
    void set_dtb_load_addr(uint32_t addr) { m_dtb_load_addr = addr; }

    void set_machine_id(uint32_t machine_id) { m_machine_id = machine_id; }
    void set_entry_blob(const PatchBlob &blob) { m_entry = PatchBlob(blob); }
    void set_secondary_entry_blob(const PatchBlob &blob) { m_secondary_entry = PatchBlob(blob); }
    void set_ram_start(uint32_t ram_start) { m_ram_start = ram_start; }

    int boot();
};

#endif
