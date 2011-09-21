/*
 *  MHP3patch kernel module
 *
 *  Copyright (C) 2011  Codestation
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "loader.h"

#include <pspsdk.h>
#include <pspkernel.h>
#include <string.h>
#include "search.h"
#include "logger.h"

#define QUEST_START_MARKER 101
#define MIB_ADDR 0x8a33e70
#define MIB_ID_SIZE 220

int sceKernelGetModel(void);
int sprintf(char *str, const char *format, ...);

static u32 *index_table;
static u32 index_elems;
static SceUID index_id;

static int quest_started = 0;
static u32 loaded_mib;
static int model_go;

static char filename[32];
static u32 k1;

static u32 *mib_table;
static u32 *quest_number;
static u32 mib_elems;

const char *mod_path[] = {
        "xxx:/mhp3rd/quest/%04d/%04d.bin",
        "xxx:/mhp3rd/extra/%04d.bin",
        "xxx:/mhp3rd/patch/%04d.bin",
};

int load_mod_index() {
    index_table = NULL;
    index_elems = 0;
    index_id = -1;
    loaded_mib = 0;

    k1 = pspSdkSetK1(0);
    model_go = sceKernelGetModel() == 4 ? 1 : 0;
    strcpy(filename, "xxx:/mhp3rd/index.bin");
    SET_DEVICENAME(filename, model_go);
    kprintf("trying to open %s\n", filename);
    SceUID fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
    if(fd < 0) {
        kprintf("Cannot find index.bin\n");
        pspSdkSetK1(k1);
        return fd;
    }
    SceSize size = (SceSize)sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);
    index_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_KERNEL, "mhp3index", PSP_SMEM_High, size, NULL);
    if(index_id >= 0) {
        index_table = sceKernelGetBlockHeadAddr(index_id);
        sceIoRead(fd, index_table, size);
        index_elems = size / sizeof(u32);
        kprintf("index size: %i bytes, entries: %i\n", size, index_elems);
    } else {
        kprintf("failed to allocate memory for table\n");
    }
    sceIoClose(fd);
    pspSdkSetK1(k1);
    return 0;
}

int load_quest_index() {
    mib_table = NULL;
    mib_elems = 0;
    k1 = pspSdkSetK1(0);
    model_go = sceKernelGetModel() == 4 ? 1 : 0;
    strcpy(filename, "xxx:/mhp3rd/quest/mib_id.dat");
    SET_DEVICENAME(filename, model_go);
    kprintf("trying to open %s\n", filename);
    SceUID fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
    if(fd < 0) {
        kprintf("Cannot find mib_id.dat\n");
        pspSdkSetK1(k1);
        return fd;
    }
    SceSize size = (SceSize)sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);
    index_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_KERNEL, "mhp3mib", PSP_SMEM_High, size, NULL);
    if(index_id >= 0) {
        mib_table = sceKernelGetBlockHeadAddr(index_id);
        sceIoRead(fd, mib_table, size);
        mib_elems = size / (sizeof(u32) * 2);
        quest_number = mib_table + mib_elems;
        kprintf("index size: %i bytes, entries: %i\n", size, index_elems);
    } else {
        kprintf("failed to allocate memory for table\n");
    }
    sceIoClose(fd);
    pspSdkSetK1(k1);
    return 0;
}

u32 *find_mod_index(u32 value) {
    kprintf("searching for %08X\n", value);
    return search_vector(value >> 11, index_table, index_elems);
}

SceUID load_mod_file(u32 number) {
    int ret = -1;
    SceIoStat stat;

    for(u32 i = 0; i < ITEMSOF(mod_path); i++) {
        if(i == 0) {
            if(loaded_mib) {
                sprintf(filename, mod_path[0], loaded_mib, number);
            } else {
                continue;
            }
        } else {
            sprintf(filename, mod_path[i], number);
        }
        SET_DEVICENAME(filename, model_go);
        kprintf("trying to open %s\n", filename);
        memset(&stat, 0, sizeof(stat));
        if(sceIoGetstat(filename, &stat) >= 0) {
            ret = sceIoOpen(filename, PSP_O_RDONLY, 0777);
            break;
        }

    }
    return ret;
}

u32 get_mod_number(u32 *pos) {
    return (u32)(pos - index_table) + 1;
}

u32 get_quest_number(u32 *pos) {
    return quest_number[(u32)(pos - mib_table)];
}


void unload_mod_index() {
    if(index_table) {
        sceKernelFreePartitionMemory(index_id);
    }
}

u32 get_address_id(u8 *address, u32 size) {
    u32 digest[5];
    sceKernelUtilsSha1Digest(address, size, (u8 *)&digest);
    return *address;
}


void quest_override(u32 mod_number) {
    u32 *result;

    if(mod_number == QUEST_START_MARKER) {
        kprintf("quest started\n");
        quest_started = 1;
        loaded_mib = 0;
    }
    if(quest_started == 1) {
        if(mod_number >= 2820 && mod_number <= 3972) {
            kprintf("detected quest: %i\n", mod_number);
            loaded_mib = mod_number;
        }
        if(mod_number > 5000) {
            if(loaded_mib == 0) {
                result = search_exact(get_address_id((u8 *)MIB_ADDR, MIB_ID_SIZE), mib_table, mib_elems);
                if(result) {
                    loaded_mib = get_quest_number(result);
                }
            }
            quest_started = 2;
        }
    }
}
