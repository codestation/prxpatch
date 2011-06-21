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

#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <string.h>
#include <psppower.h>
#include "sceio.h"
#include "data_install.h"
#include "misc.h"
#include "logger.h"

#define DATABIN_PATH "disc0:/PSP_GAME/USRDIR/DATA.BIN"
#define SUSPEND_FUNCNAME "power"
#define MAX_PATCHFILES 256

u32 sctrlHENFindFunction(char *modname, char *libname, u32 nid);

// offset and size tables container
SceSize patch_offset[MAX_PATCHFILES];
unsigned int patch_size[MAX_PATCHFILES];

// number of tables
unsigned int patch_count;

// starting offset of the data
SceSize data_start = 0;

// file descriptors of the data.bin and translation data
SceUID datafd = -1;

// semaphore to avoid reading on a closed file
SceUID io_sema = -1;

// the PSP can send 2 suspend events, but we need to check only one
int suspending = 0;

// the translation data needs to be (re)opened
int repoen = 1;

int k1;

SceUID (*sceIoOpen_func)(const char *file, int flags, SceMode mode) = NULL;
int (*sceIoRead_func)(SceUID fd, void *data, SceSize size) = NULL;
int (*sceIoLseek32_func)(SceUID fd, int offset, int whence) = NULL;
int (*sceIoClose_func)(SceUID fd) = NULL;

void set_calls() {
    sceIoOpen_func =  (void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x109F50BC);
    sceIoRead_func =  (void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x6A638D83);
    sceIoLseek32_func = (void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x68963324);
    sceIoClose_func = (void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x810C4BC3);
}

void fill_tables(SceUID fd) {
    if(fd < 0)
        return;
    sceIoLseek32(fd, 0, PSP_SEEK_SET);
    sceIoRead(fd, &patch_count, 4);
    if(patch_count > MAX_PATCHFILES)
        patch_count = MAX_PATCHFILES;
    unsigned int i;
    for(i = 0; i < patch_count; i++) {
        sceIoRead(fd, &patch_offset[i], 4);
        sceIoRead(fd, &patch_size[i], 4);
    }
    data_start = ((patch_count + 1) * 8);
    if(data_start % 16 > 0)
        data_start += 16 - (data_start % 16);
}

SceUID mhp3_open(const char *file, int flags, SceMode mode) {
    if(!sceIoOpen_func)
        set_calls();
    SceUID fd = sceIoOpen_func(file, flags, mode);
    if(fd >= 0) {
        k1 = pspSdkSetK1(0);
        if(strcmp(file, DATABIN_PATH) == 0) {
            reopen_translation();
            fill_tables(transfd);
            fill_install_tables(transfd);
            datafd = fd;
        } else {
            register_install(file, fd);
        }
        pspSdkSetK1(k1);
    }
    return fd;
}

int mhp3_read(SceUID fd, void *data, SceSize size) {
    int res;
    if(fd == datafd) {
        SceSize pos = sceIoLseek32_func(fd, 0, PSP_SEEK_CUR);
        unsigned int i = 0;
        SceSize offset = data_start;
        while(i < patch_count) {
            if(pos < patch_offset[i] + patch_size[i] && pos + size > patch_offset[i]) {
                k1 = pspSdkSetK1(0);
                reopen_translation();
                sceIoLseek32(transfd, offset + (pos - patch_offset[i]), PSP_SEEK_SET);
                res = sceIoRead(transfd, data, size);
                if(res != size) {
                    logger("failed to read translation data\n");
                }
                pspSdkSetK1(k1);
                sceIoLseek32_func(fd, size, PSP_SEEK_CUR);
                return res;
            }
            offset += patch_size[i];
            ++i;
        }
    } else {
        res = read_install(fd, data, size);
        return res;
    }
    res = sceIoRead_func(fd, data, size);
    return res;
}

int mhp3_close(SceUID fd) {
    if(fd == datafd) {
		datafd = -1;
	} else {
	    unregister_install(fd);
	}
    int res = sceIoClose_func(fd);
    return res;
}
