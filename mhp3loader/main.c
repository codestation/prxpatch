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

#include "main.h"

#include <pspkernel.h>
#include <string.h>

#include "hook.h"
#include "reader.h"
#include "logger.h"

PSP_MODULE_INFO("mhp3patch", PSP_MODULE_KERNEL, 1, 8);

#define GAME_MODULE "MonsterHunterPortable3rd"

stub stubs[] = {
        { 0x109F50BC, mhp3_open  }, // sceIoOpen
        { 0x6A638D83, mhp3_read  }, // sceIoRead
        { 0x810C4BC3, mhp3_close }, // sceIoClose
};

STMOD_HANDLER previous = NULL;
SceUID sema = 0;

inline void patch_imports(SceModule *module) {
    for (u32 i = 0; i < ITEMSOF(stubs); i++) {
        if(hook_import_bynid(module, "IoFileMgrForUser", stubs[i].nid, stubs[i].func, 1) < 0) {
            kprintf("Failed to hook %08X\n", stubs[i].nid);
        }
    }
}

int module_start_handler(SceModule *module) {
    if ((module->text_addr & 0x80000000) != 0x80000000 && strcmp(module->modname, GAME_MODULE) == 0) {
        sceKernelSignalSema(sema, 1);
    }
    return previous ? previous(module) : 0;
}

int thread_start(SceSize args UNUSED, void *argp UNUSED) {
    sema = sceKernelCreateSema("mhp3patch_wake", 0, 0, 1, NULL);
    previous = sctrlHENSetStartModuleHandler(module_start_handler);
    sceKernelWaitSema(sema, 1, NULL);
    SceModule *module = sceKernelFindModuleByName(GAME_MODULE);
    if(module) {
        patch_imports(module);
    }
    sceKernelDeleteSema(sema);
    return 0;
}

int module_start(SceSize args UNUSED, void *argp UNUSED) {
    SceUID thid = sceKernelCreateThread("mhp3patch_main", thread_start, 0x22, 0x2000, 0, NULL);
    if(thid >= 0) {
        sceKernelStartThread(thid, 0, NULL);
    }
    return 0;
}

int module_stop(SceSize args UNUSED, void *argp UNUSED) {
    return 0;
}
