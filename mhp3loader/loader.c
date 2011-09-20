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

#include <pspkernel.h>
#include <string.h>
#include "search.h"
#include "logger.h"

#define MOD_START_MARKER 101

int sceKernelGetModel(void);
int sprintf(char *str, const char *format, ...);

static u32 *index_table = NULL;
static u32 index_elems = 0;
static SceUID index_id = -1;

static int mod_redirect = -1;
static u32 loaded_mib = 0;
static int model_go;

char filename[32];

const char *mod_path[] = {
        "xxx:/mhp3rd/mods/%04d/%04d.bin",
        "xxx:/mhp3rd/extra/%04d.bin",
        "xxx:/mhp3rd/patch/%04d.bin",
};

int load_mod_index() {
    model_go = sceKernelGetModel() == 4 ? 1 : 0;
    strcpy(filename, "xxx:/mhp3rd/index.bin");
    SET_DEVICENAME(filename, model_go);
    SceUID fd = sceIoOpen(filename, PSP_O_RDONLY, 0644);
    if(fd < 0) {
        kprintf("Cannot find index.bin\n");
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
    return 0;
}

u32 *find_mod_index(u32 value) {
    return search_vector(value >> 11, index_table, index_elems);
}

SceUID load_mod_file(u32 number) {
    int ret;

    for(u32 i = 0; i < ITEMSOF(mod_path); i++) {
        if(i == 0 && loaded_mib) {
            sprintf(filename, mod_path[0], loaded_mib, number);
        } else {
            sprintf(filename, mod_path[i], number);
        }
        SET_DEVICENAME(filename, model_go);
        ret = sceIoOpen(filename, PSP_O_RDONLY, 0644);
        if(ret >= 0) {
            break;
        }
    }
    return ret;
}

u32 get_mod_number(u32 *pos) {
    return (u32)(pos - index_table);
}

void unload_mod_index() {
    if(index_table) {
        sceKernelFreePartitionMemory(index_id);
    }
}

static int mod_check(u32 mod_number) {
    return mod_number == 3668 ? 1 : 0;
}

int need_redirection(u32 mod_number) {
    if(mod_redirect <= 0) {
        if(mod_number == MOD_START_MARKER) {
            mod_redirect = 0;
        } else if(mod_redirect == 0) {
            // check for the loaded mib if any
            if(mod_number >= 2820 && mod_number <= 3972) {
                loaded_mib = mod_number;
            }
            if(mod_number > 5000) {
                mod_redirect = mod_check(mod_number);
            }
        }
    }
    return mod_redirect > 0 ? 1 : 0;
}

