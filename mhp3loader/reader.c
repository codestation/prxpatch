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

#include "reader.h"
#include "loader.h"
#include "logger.h"

#include <pspsysmem.h>
#include <pspiofilemgr.h>
#include <string.h>

#define DATABIN_PATH "disc0:/PSP_GAME/USRDIR/DATA.BIN"

static SceUID datafd = -1;
static u32 k1;

SceUID mhp3_open(const char *file, int flags, SceMode mode) {
    SceUID fd = sceIoOpen(file, flags, mode);
    if (fd >= 0) {
        if (strcmp(file, DATABIN_PATH) == 0) {
            kprintf("DATA.BIN opened\n");
            load_mod_index();
            datafd = fd;
        }
    }
    return fd;
}

int mhp3_read(SceUID fd, void *data, SceSize size) {
    u32 file_offset;
    u32 *file_index;
    u32 mod_number;
    SceUID modfd;
    int res;

    if (fd == datafd) {
        file_offset = (u32)sceIoLseek(fd, 0, PSP_SEEK_CUR);
        file_index = find_mod_index(file_offset);
        if(file_index) {
            kprintf("index found: %08X\n", *file_index);
            mod_number = get_mod_number(file_index);
            kprintf("translated file number: %i\n", mod_number);
            if(mod_number > 0) {
                pspSdkSetK1(0);
                quest_override(mod_number);
                modfd = load_mod_file(mod_number);
                if(modfd >= 0) {
                    sceIoLseek(modfd, file_offset - (*file_index << 11), PSP_SEEK_SET);
                    kprintf("reading offset %08X, size: %08X\n", file_offset, size);
                    res = sceIoRead(modfd, data, size);
                    sceIoClose(modfd);
                    kprintf("read returned %08X\n", res);
                    sceIoLseek(fd, res, PSP_SEEK_CUR);
                    pspSdkSetK1(k1);
                    return res;
                }
                pspSdkSetK1(k1);
            }
        }
    }
    return sceIoRead(fd, data, size);
}

int mhp3_close(SceUID fd) {
    if (fd == datafd) {
        unload_mod_index();
        datafd = -1;
    }
    return sceIoClose(fd);
}
