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

#include "rabbits/loader/bootloader.h"
#include "rabbits/loader/loader.h"

#include "rabbits/logger.h"

ArmBootloader::PatchBlob::PatchBlob(const Entry blob[], size_t size)
{
    size_t i;

    for (i = 0; i < size; i++) {
        m_blob.push_back(blob[i]);
    }
}

void ArmBootloader::PatchBlob::patch(const uint32_t ctx[NUM_FIXUP])
{
    std::vector<Entry>::iterator it;

    for (it = m_blob.begin(); it != m_blob.end(); it++) {
         switch (it->fixup) {
         case FIXUP_NONE:
         case NUM_FIXUP:
             break;

         default:
             it->insn = ctx[it->fixup];
             break;
         }
    }
}

int ArmBootloader::PatchBlob::load(uint32_t addr, DebugInitiator *bus)
{
    std::vector<Entry>::iterator it;
    
    for (it = m_blob.begin(); it != m_blob.end(); it++) {
        if(bus->debug_write(addr, &(it->insn), 4) < 4) {
            LOG_F(APP, ERR, "Unable to write entry blob. Trying to write outside ram?\n");
            return 1;
        }
        addr += 4;
    }

    return 0;
}

ArmBootloader::ArmBootloader(DebugInitiator *bus)
{
    m_bus = bus;
    m_ram_start = 0;
    m_kernel_load_addr = m_initramfs_load_addr = m_dtb_load_addr = -1;
}

ArmBootloader::~ArmBootloader()
{
}

int ArmBootloader::boot()
{
    bool have_dtb = false;

    uint32_t dtb_load_addr, initramfs_load_addr, kernel_load_addr;
    uint32_t boot_data = 0, kernel_entry = 0;
    uint64_t dtb_size;

    uint32_t patch_ctx[NUM_FIXUP];

    /* Device tree */
    if (!m_dtb_path.empty()) {
        LOG_F(APP, DBG, "Loading dtb %s\n", m_dtb_path.c_str());

        if (m_dtb_load_addr != (uint32_t) -1) {
            dtb_load_addr = m_dtb_load_addr;
        } else {
            dtb_load_addr = DTB_DEFAULT_LOAD_ADDR + m_ram_start;
        }

        if (Loader::load_image(m_dtb_path, m_bus, dtb_load_addr, &dtb_size)) {
            LOG_F(APP, ERR, "Unable to load dtb %s\n", m_dtb_path.c_str());
            return 1;
        }
        have_dtb = true;
    }

    /* Initramfs */
    if (!m_initramfs_path.empty()) {
        LOG_F(APP, DBG, "Loading initramfs %s\n", m_initramfs_path.c_str());

        if (m_initramfs_load_addr != (uint32_t) -1) {
            initramfs_load_addr = m_initramfs_load_addr;
        } else {
            initramfs_load_addr = DTB_DEFAULT_LOAD_ADDR + m_ram_start;
            if(have_dtb) {
                initramfs_load_addr += dtb_size;
            }
        }

        if (Loader::load_image(m_initramfs_path, m_bus, initramfs_load_addr, NULL)) {
            LOG_F(APP, ERR, "Unable to load initramfs %s\n", m_initramfs_path.c_str());
            return 1;
        }
    }

    /* Kernel */
    if (!m_kernel_path.empty()) {
        LOG_F(APP, DBG, "Loading kernel %s\n", m_kernel_path.c_str());

        struct stat buf;
        if(stat(m_kernel_path.c_str(), &buf) != 0) {
            LOG_F(APP, ERR, "kernel image file not found : %s\n", m_kernel_path.c_str());
            exit(1);
        }

        /* Try to load it as an ELF first 
         * In this case, the load address is contained in the image */
        uint64_t kernel_entry64;

        LOG_F(APP, DBG, "Trying ELF...\n");
        int is_not_elf = Loader::load_elf(m_kernel_path, m_bus, &kernel_entry64);

        if(is_not_elf) {
            LOG_F(APP, DBG, "ELF failed, loading as binary image\n");

            if (m_kernel_load_addr != (uint32_t) -1) {
                kernel_load_addr = m_kernel_load_addr;
            } else {
                kernel_load_addr = KERNEL_DEFAULT_LOAD_ADDR + m_ram_start;
            }

            kernel_entry = kernel_load_addr;

            if(Loader::load_image(m_kernel_path, m_bus, kernel_load_addr, NULL)) {
                LOG_F(APP, ERR, "Unable to load kernel %s\n", m_kernel_path.c_str());
                return 1;
            }
        } else {
            kernel_entry = kernel_entry64;
        }
    }

    /* Entry blobs patching and loading */
    if (have_dtb) {
        boot_data = dtb_load_addr;
    }

    patch_ctx[FIXUP_MACHINE_ID] = m_machine_id;
    patch_ctx[FIXUP_BOOT_DATA] = boot_data;
    patch_ctx[FIXUP_KERNEL_ENTRY] = kernel_entry;
    patch_ctx[FIXUP_SMP_BOOTREG] = 0;
    patch_ctx[FIXUP_SECONDARY_ENTRY] = m_entry.size();

    if (!m_secondary_entry.empty()) {
        LOG_F(APP, DBG, "Loading secondary entry blob\n");
        m_secondary_entry.patch(patch_ctx);
        m_secondary_entry.load(m_entry.size(), m_bus);
    }

    if (!m_entry.empty()) {
        LOG_F(APP, DBG, "Loading entry blob\n");
        m_entry.patch(patch_ctx);
        m_entry.load(0, m_bus);
    }

    return 0;
}
