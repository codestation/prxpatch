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

#include <pspkernel.h>
#include <string.h>
#include "libs.h"
#include "sceio.h"
#include "misc.h"
#include "logger.h"

PSP_MODULE_INFO("mhp3patch", PSP_MODULE_KERNEL, 1, 5);

#define GAME_MODULE "MonsterHunterPortable3rd"

typedef int (* STMOD_HANDLER)(SceModule *);

STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler);

STMOD_HANDLER previous = NULL;

SceUID sema = 0;

void *functions[] = {
        mhp3_open,
        mhp3_read,
        mhp3_seek,
        mhp3_close,
        mhp3_callback,
};

unsigned int nids[] = {
        0x109F50BC, // sceIoOpen
        0x6A638D83, // sceIoRead
        0x27EB27B8, // sceIoLseek
        0x810C4BC3, // sceIoClose
        0xE81CAF8F, // sceKernelCreateCallback
};

void patch_imports(SceModule *module) {
    const char *base = "IoFileMgrForUser";
    for(int i = 0;i < (sizeof(nids) / 4); i++) {
        if(i == 4)
            base = "ThreadManForUser";
        else if (i == 5) {
            base = "sceUtility";
        }
        hook_import_bynid(module, base, nids[i], functions[i], 1);
    }
}

int module_start_handler(SceModule *module) {
    if((strcmp(module->modname, GAME_MODULE) == 0) && (module->text_addr & 0x80000000) != 0x80000000) {
        sceKernelSignalSema(sema, 1);
    }
    return previous ? previous(module) : 0;
}

int thread_start(SceSize args, void *argp) {
    sema = sceKernelCreateSema("mhp3patch_wake", 0, 0, 1, NULL);
    previous = sctrlHENSetStartModuleHandler(module_start_handler);
    sceKernelWaitSema(sema, 1, NULL);
    SceModule *module = sceKernelFindModuleByName(GAME_MODULE);
    if(module)
        patch_imports(module);
    sceKernelDeleteSema(sema);
    return 0;
}

int module_start(SceSize argc, void *argp) {
    SceUID thid = sceKernelCreateThread("mhp3patch_main", thread_start, 0x22, 0x2000, 0, NULL);
    if(thid >= 0)
        sceKernelStartThread(thid, argc, argp);
    return 0;
}

int module_stop(SceSize args, void *argp) {
    if(previous)
        sctrlHENSetStartModuleHandler(previous);
	return 0;
}
