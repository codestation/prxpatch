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
#include "systemctrl.h"
#include "logger.h"

PSP_MODULE_INFO("pjd2patch", PSP_MODULE_KERNEL, 1, 0);
PSP_HEAP_SIZE_KB(0);

#define GAME_ID "ULJM05681"
#define GAME_MODULE "PdvApp"
#define TRANS_FILE_MS "pjd2_translation.bin"
#define TRANS_FILE_GO "pjd2_translation.bin"
#define MODEL_PSPGO 4

int sceKernelGetModel();

SceUID sema = 0;
SceUID sema_start = 0;
STMOD_HANDLER previous = NULL;
int model = -1;

SceUID block_id = -1;
void *block_addr = NULL;

struct addr_data {
    void **addr;
    int offset;
}__attribute__((packed));

void patch_eboot(SceModule2 *module)  {
    SceUID fd = sceIoOpen(model == MODEL_PSPGO ? TRANS_FILE_GO : TRANS_FILE_MS, PSP_O_RDONLY, 0777);
    if(fd < 0) {
        return;
    }
    int count;
    SceSize size = sceIoLseek32(fd, 0, PSP_SEEK_END);
    sceIoLseek32(fd, 0, PSP_SEEK_SET);
    sceIoRead(fd, &count, 4);
    int index_size = (count * 8) + 4;
    index_size += 16 - (index_size % 16);
    int table_size = size - index_size;
    block_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "pjd2-trans", PSP_SMEM_High, table_size, NULL);
    if(block_id < 0) {
        return;
    }
    block_addr = sceKernelGetBlockHeadAddr(block_id);
    for(int i = 0; i < count; i++) {
        struct addr_data data;
        sceIoRead(fd, &data, sizeof(data));
        *data.addr = block_addr + data.offset;
    }
    sceIoLseek32(fd, index_size, PSP_SEEK_SET);
    sceIoRead(fd, block_addr, table_size);
    sceIoClose(fd);
}

int module_start_handler(SceModule2 * module) {
    if(strcmp(module->modname, GAME_MODULE) == 0 &&
            (module->text_addr & 0x80000000) != 0x80000000) {
        sceKernelSignalSema(sema, 1);
        sceKernelWaitSema(sema_start, 1, NULL);
    }
    return previous ? previous(module) : 0;
}

int thread_start(SceSize args, void *argp) {
    model = sceKernelGetModel();
    sema = sceKernelCreateSema("pjd2patch_wake", 0, 0, 1, NULL);
    sema_start = sceKernelCreateSema("pjd2patch_start", 0, 0, 1, NULL);
    previous = sctrlHENSetStartModuleHandler(module_start_handler);
    sceKernelWaitSema(sema, 1, NULL);
    SceModule2 *module = (SceModule2*)sceKernelFindModuleByName(GAME_MODULE);
    if(module) {
        patch_eboot(module);
        sceKernelSignalSema(sema_start, 1);
        sceKernelDelayThread(10000);
        sceKernelDeleteSema(sema);
        sceKernelDeleteSema(sema_start);
    }
    sceKernelExitDeleteThread(0);
    return 0;
}

int module_start(SceSize argc, void *argp) {
	SceUID thid = sceKernelCreateThread("pjd2patch_main", thread_start, 0x22, 0x2000, 0, NULL);
	if(thid >= 0)
		sceKernelStartThread(thid, argc, argp);
	return 0;
}

int module_stop(SceSize args, void *argp) {
    if(block_id >= 0) {
        sceKernelFreePartitionMemory(block_id);
    }
	return 0;
}
