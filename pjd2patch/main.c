/*
 *  PJD2Patch kernel module
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

#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <string.h>
#include "logger.h"

PSP_MODULE_INFO("pjd2patch", PSP_MODULE_KERNEL, 1, 1);
PSP_HEAP_SIZE_KB(0);

#define GAME_ID "ULJM05681"
#define GAME_MODULE "PdvApp"
#define TRANS_FILE "pjd2_translation.bin"

#define UNUSED __attribute__((unused))

typedef int (* STMOD_HANDLER)(SceModule *);

STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler);

char filepath[256];

STMOD_HANDLER previous = NULL;

SceUID block_id = -1;
void *block_addr = NULL;

struct addr_data {
    void **addr;
    u32 offset;
}__attribute__((packed));

void patch_eboot()  {
    u32 count;
    SceUID fd;
    SceOff size;
    SceSize index_size, table_size;

    strrchr(filepath, '/')[1] = 0;
    strcat(filepath, TRANS_FILE);
    fd = sceIoOpen(filepath, PSP_O_RDONLY, 0777);
    if(fd < 0) {
        return;
    }
    size = sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);
    sceIoRead(fd, &count, 4);
    index_size = (count * 8) + 4;
    index_size += 16 - (index_size % 16);
    table_size = (SceSize)size - index_size;
    block_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "pjd2-trans", PSP_SMEM_High, table_size, NULL);
    if(block_id < 0) {
        return;
    }
    block_addr = sceKernelGetBlockHeadAddr(block_id);
    for(u32 i = 0; i < count; i++) {
        struct addr_data data;
        sceIoRead(fd, &data, sizeof(data));
        void *final_addr = (void *)((u32)(block_addr) + data.offset);
        if((u32)data.addr & 0xF0000000) {
            data.addr = (void **)(((u32)data.addr) & ~0xF0000000);
            data.addr = (void **)(((u32)data.addr) | 0x40000000);
            u16 *code_addr = (u16 *)data.addr;
            u16 addr1 = (u32)final_addr & 0xFFFF;
            u16 addr2 = (u16)((((u32)final_addr) >> 16) & 0xFFFF);
            if(addr1 & 0x8000) {
                addr2++;
            }
            int j = 0;
            while(j < 32) { //maximum backtrace to look for a lui instruction
                if((*(code_addr - 3 - (j*2)) & 0x3C00) == 0x3C00) { // lui
                    *code_addr = addr1;
                    code_addr -= (4 + (j*2));
                    *code_addr = addr2;
                    break;
                } else {
                    j++;
                }
            }
            if(j >= 32) {
                kprintf("Backtrace failed, cannot find matching lui for %08X\n", (u32)data.addr);
            }
        } else {
            data.addr = (void **)(((u32)data.addr) | 0x40000000);
            *data.addr = final_addr;
        }
    }
    sceIoLseek(fd, index_size, PSP_SEEK_SET);
    sceIoRead(fd, block_addr, table_size);
    sceIoClose(fd);
}

int module_start_handler(SceModule * module) {
    if((module->text_addr & 0x80000000) != 0x80000000 && strcmp(module->modname, GAME_MODULE) == 0) {
        patch_eboot();
    }
    return previous ? previous(module) : 0;
}

int module_start(SceSize argc UNUSED, void *argp) {
    strcpy(filepath, argp);
    previous = sctrlHENSetStartModuleHandler(module_start_handler);
	return 0;
}

int module_stop(SceSize args UNUSED, void *argp UNUSED) {
    if(block_id >= 0) {
        sceKernelFreePartitionMemory(block_id);
    }
	return 0;
}
