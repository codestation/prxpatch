/*
 *  divapatch kernel module
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
#include "search.h"
#include "logger.h"

#define ITEMSOF(arr) (int)(sizeof(arr) / sizeof(0[arr]))

#define DIVACPK_PATH "disc0:/PSP_GAME/USRDIR/media/afs/Diva2ExData.cpk"

typedef struct {
    u32 offset;
    u32 size;
    const char *filename;
} node;

u32 cpk_offsets[] = { 0xDB800, 0xE0000, 0x7F4000 };
u32 cpkfile_sizes[] = { 15376, 14494,  4904 };
const char *cpk_filenames[] = { "menu_help_01.png", "menu_help_02.png", "menu_title_home.png" };

SceUID datafd;

extern char filepath[256];

SceUID diva_open(const char *file, int flags, SceMode mode) {
    SceUID fd = sceIoOpen(file, flags, mode);
    if (fd >= 0) {
        if (strcmp(file, DIVACPK_PATH) == 0) {
            kprintf("diva2exdata.cpk opened\n");
            datafd = fd;
        }
    }
    return fd;
}

static SceUID open_file(const char *file) {
    int ret = -1;
    SceIoStat stat;

    strrchr(filepath, '/')[1] = 0;
    strcat(filepath, file);
    kprintf("trying to open %s\n", filepath);
    memset(&stat, 0, sizeof(stat));
    if(sceIoGetstat(filepath, &stat) >= 0) {
        ret = sceIoOpen(filepath, PSP_O_RDONLY, 0777);
    }
    return ret;
}

int diva_read(SceUID fd, void *data, SceSize size) {
    u32 file_offset;
    u32 *file_index;
    u32 vector_pos;
    SceUID modfd;
    int res;
    u32 k1;

    if (fd == datafd) {
        file_offset = (u32)sceIoLseek(fd, 0, PSP_SEEK_CUR);
        file_index = search_exact(file_offset, cpk_offsets, ITEMSOF(cpk_offsets));
        if(file_index != NULL) {
            kprintf("index found: %08X\n", *file_index);
            vector_pos = (u32)(file_index - cpk_offsets);
            kprintf("translated vector pos: %i\n", vector_pos);
            k1 = pspSdkSetK1(0);
            modfd = open_file(cpk_filenames[vector_pos]);
            if(modfd >= 0) {
                kprintf("reading offset %08X, size: %08X\n", file_offset, size);
                res = sceIoRead(modfd, data, size);
                sceIoClose(modfd);
                kprintf("read returned %08X\n", res);
                res = (int)size;
                sceIoLseek(fd, size, PSP_SEEK_CUR);
                pspSdkSetK1(k1);
                return res;
            }
            pspSdkSetK1(k1);
        }
    }
    return sceIoRead(fd, data, size);
}

int diva_close(SceUID fd) {
    if (fd == datafd) {
        datafd = -1;
    }
    return sceIoClose(fd);
}
