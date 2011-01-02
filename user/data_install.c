/*
 *  MHP3patch user module: data install patcher
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

#include <pspiofilemgr.h>
#include <pspthreadman.h>
#include <string.h>
#include "data_install.h"

#define TRANSLATION_PATH "ms0:/MHP3RD_DATA.BIN"

//TODO: move these tables to patchfile and make it dynamic

// data install files: 0011, 0031, 0032, 0033
unsigned int install_id[] = {0x31313030, 0x31333030, 0x32333030, 0x33333030};

//position where it starts a new offset group
const unsigned int install_pos[] = {0, 1, 6, 16};

// data install fd's
SceUID install_fd[4] = {-1, -1, -1, -1};

// data install offsets in files, for now it needs
// 0017 2813 2814 2816 2817 2818 0098
// 3973 3974 3975 3976 3977 3978 3979 3980 3984 3985 3986 3987
// in the translation datafile to work, this gonna go away when
// i'll move it to the .BIN file
const SceSize install_offset[] = {0x00204000, 0x00280800, 0x00284800, 0x002AE000,
                            0x002D1800, 0x00331000, 0x0033F800, 0x00341800,
                            0x00346000, 0x0034B800, 0x00351000, 0x00356800,
                            0x0035C000, 0x00361000, 0x00367000, 0x00369000,
                            0x00000000, 0x00001000};

void register_install(const char *file, SceUID fd) {
    // ms0:\.\PSP\SAVEDATA\ULJM05800DAT\000000XX  | length: 41
    // if(strstr(file,"ULJM05800DAT")) {  //idk why strstr isn't working here :/
    if(strlen(file) == 41) {
        unsigned int i = 0;
        while(i < sizeof(install_id) / 4) {
            const char *offset = file + strlen(file) - 4;
            if(!memcmp(offset, &install_id[i], 4)) {
                install_fd[i] = fd;
                break;
            }
            i++;
        }
    }
}

void unregister_install(SceUID fd) {
    unsigned int i = 0;
    while(i < sizeof(install_id) / 4) {
        if(install_fd[i] == fd) {
            install_fd[i] = -1;
            break;
        }
        i++;
    }
}

int read_install(SceUID fd, void *data, SceSSize size) {
    unsigned int i = 0;
    while(i < sizeof(install_fd) / 4) {
        if(install_fd[i] == fd) {
            SceSize pos = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
            SceSize offset = data_start;
            unsigned int j = install_pos[i];
            unsigned int l = i < ((sizeof(install_pos) / 4) - 1)? install_pos[i+1] : (sizeof(install_offset) / 4);
            while(j < l) {
                if(pos < install_offset[j] + patch_size[j] && pos + size > install_offset[j]) {
                    unsigned int k;
                    for(k = 0; k < j; k++)
                        offset += patch_size[k];
                    sceKernelWaitSema(sema, 1, NULL);
                    transfd = sceIoOpen(TRANSLATION_PATH, PSP_O_RDONLY, 0777);
                    sceIoLseek32(transfd, offset + (pos - install_offset[j]), PSP_SEEK_SET);
                    int res = sceIoRead(transfd, data, size);
                    sceIoClose(transfd);
                    sceIoLseek32(fd, size, PSP_SEEK_CUR);
                    sceKernelSignalSema(sema, 1);
                    return res;
                }
                j++;
            }
        }
        i++;
    }
    return -1;
}
