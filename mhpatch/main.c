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

#include "libs.h"
#include "sceio.h"
#include "misc.h"
#include "logger.h"

#include <pspkernel.h>
#include <string.h>

PSP_MODULE_INFO("mhp3patch", PSP_MODULE_KERNEL, 1, 8);

#define ITEMSOF(arr) (sizeof(arr) / sizeof(0[arr]))

#define GAME_MODULE "MonsterHunterPortable3rd"

#define UNUSED __attribute__((unused))

typedef int (*STMOD_HANDLER)(SceModule *);

typedef struct {
    u32 nid;
    void *func;
} stub;

STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler);

STMOD_HANDLER previous = NULL;

stub stubs[] = {
        { 0x109F50BC, mhp3_open },  // sceIoOpen
        { 0x6A638D83, mhp3_read },  // sceIoRead
        { 0x810C4BC3, mhp3_close }, // sceIoClose
};

inline void patch_imports(SceModule *module) {
    for (u32 i = 0; i < ITEMSOF(stubs); i++) {
        hook_import_bynid(module, "IoFileMgrForUser", stubs[i].nid, stubs[i].func, 1);
    }
}

int module_start_handler(SceModule *module) {
    if ((strcmp(module->modname, GAME_MODULE) == 0) && (module->text_addr & 0x80000000) != 0x80000000) {
        patch_imports(module);
    }
    return previous ? previous(module) : 0;
}

int module_start(SceSize args UNUSED, void *argp UNUSED) {
    previous = sctrlHENSetStartModuleHandler(module_start_handler);
    return 0;
}

int module_stop(SceSize args UNUSED, void *argp UNUSED) {
    return 0;
}
