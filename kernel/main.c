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
#include <string.h>
#include "libs.h"
#include "misc.h"
#include "logger.h"

PSP_MODULE_INFO("pjd2patch", PSP_MODULE_KERNEL, 1, 0);
PSP_HEAP_SIZE_KB(0);

#define GAME_ID "ULJM05681"
#define GAME_MODULE "xxxxxxxxxx"

SceUID sema = 0;
STMOD_HANDLER previous = NULL;

char * sceKernelGetUMDData(void);

void patch_eboot(SceModule2 *module)  {
}

int module_start_handler(SceModule2 * module) {
    if(strcmp(sceKernelGetUMDData() + 0x44, GAME_ID) == 0 &&
            strcmp(module->modname, GAME_MODULE) == 0 &&
            (module->text_addr & 0x80000000) != 0x80000000) {
        sceKernelSignalSema(sema, 1);
        sceKernelWaitSemaCB(sema, 0, NULL);
    }
    return previous ? previous(module) : 0;
}

int thread_start(SceSize args, void *argp) {
    if(strcmp(sceKernelGetUMDData() + 0x44, GAME_ID) == 0) {
        sema = sceKernelCreateSema("pjd2patch_wake", 0, 0, 1, NULL);
        previous = sctrlHENSetStartModuleHandler(module_start_handler);
        sceKernelWaitSemaCB(sema, 1, NULL);
        SceModule2 *module = (SceModule2*)sceKernelFindModuleByName(GAME_MODULE);
        if(module) {
            patch_eboot(module);
            sceKernelSignalSema(sema, 0);
            sceKernelDelayThread(10000);
            sceKernelDeleteSema(sema);
        }
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
	return 0;
}
