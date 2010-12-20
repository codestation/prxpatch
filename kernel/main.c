/*
 *  MHP3patch kernel module
 *
 *  Copyright (C) 2010  Codestation
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
//#include "systemctrl_se.h"
#include "libs.h"
#include "logger.h"

PSP_MODULE_INFO("mhp3patch", PSP_MODULE_KERNEL, 1, 0);
PSP_HEAP_SIZE_KB(0);

#define GAME_ID "ULJM05800"
#define GAME_MODULE "MonsterHunterPortable3rd"

int running = 0;
SceUID sema = 0;
STMOD_HANDLER previous = NULL;

char * sceKernelGetUMDData(void);

void *functions[3] = {0, 0, 0};

void registerfunctions(void * userfunctions[3]) {
    memcpy(functions, userfunctions, sizeof(functions));
    running = 1;
}

void patch_io(SceModule2 *module) {
    const char * base = "IoFileMgrForUser";
    hook_import_bynid(module, base, 0x109F50BC, functions[0], 0);
    hook_import_bynid(module, base, 0x6A638D83, functions[1], 0);
    hook_import_bynid(module, base, 0x810C4BC3, functions[2], 0);
}

int module_start_handler(SceModule2 * module) {
    if(strcmp(sceKernelGetUMDData() + 0x44, GAME_ID) == 0 && strcmp(module->modname, GAME_MODULE) == 0 && (module->text_addr & 0x80000000) != 0x80000000) {
        sceKernelSignalSema(sema, 1);
    }
    return previous ? previous(module) : 0;
}

int thread_start(SceSize args, void *argp) {
    // basic UMD-only check, requires systemctrl_se.h as header and
    // libpspsystemctrl_kernel.a from the M33 SDK
    //
    //char *iso = sctrlSEGetUmdFile();
    //if(iso) {
    //    sceKernelExitDeleteThread(0);
    //    return 0;
    //}
    if(strcmp(sceKernelGetUMDData() + 0x44, GAME_ID) == 0) {
        sema = sceKernelCreateSema("mhp3patch_wake", 0, 0, 1, NULL);
        previous = sctrlHENSetStartModuleHandler(module_start_handler);
        sceKernelWaitSemaCB(sema, 1, NULL);
        sceKernelDeleteSema(sema);
        SceUID user = -1;
        {
            char usermodule[256];
            strcpy(usermodule, "ms0:/seplugins/");
            if(argp) {
                strcpy(usermodule, (char*)argp);
                strrchr(usermodule, '/')[1] = 0;
            }
            strcpy(usermodule + strlen(usermodule), "mhp3patch_user.prx");
            struct SceKernelLMOption opt; memset(&opt, 0, sizeof(opt));
            opt.size = sizeof(opt); opt.position = 1;
            user = sceKernelLoadModule(usermodule, 0, &opt);
        }
        int status = 0;
        int start = sceKernelStartModule(user, 0, NULL, &status, NULL);
        if(start >= 0) {
            while(!running) {
                sceKernelDelayThread(10000);
            }
        }
        if(running) {
            SceModule2 *module = (SceModule2*)sceKernelFindModuleByName(GAME_MODULE);
            if(module) {
                patch_io(module);
            }
        }
    }
    sceKernelExitDeleteThread(0);
    return 0;
}

int module_start(SceSize argc, void *argp) {
	SceUID thid = sceKernelCreateThread("mhp3patch_main", thread_start, 0x22, 0x2000, 0, NULL);
	if(thid >= 0)
		sceKernelStartThread(thid, argc, argp);
	return 0;
}

int module_stop(SceSize args, void *argp) {
	running = 0;
	return 0;
}
