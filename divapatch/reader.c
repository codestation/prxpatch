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
#include <pspsysmem.h>
#include <pspiofilemgr.h>
#include <string.h>
#include "search.h"
#include "reader.h"
#include "logger.h"

#define ITEMSOF(arr) (int)(sizeof(arr) / sizeof(0[arr]))

const char *cpk_pack[] = {
        "disc0:/PSP_GAME/USRDIR/media/afs/DivaDataList.afs",
        "disc0:/PSP_GAME/USRDIR/media/afs/Diva2Data.cpk",
        "disc0:/PSP_GAME/USRDIR/media/afs/Diva2ExData.cpk",
        "disc0:/PSP_GAME/USRDIR/media/afs/Diva2ExData.cpk",
        "disc0:/PSP_GAME/USRDIR/media/afs/Diva2Data.cpk",
        "disc0:/PSP_GAME/USRDIR/media/afs/Diva2Data.cpk",
};

const char *image_files[] = {
        "diva1st_images.bin",
        "diva2nd_images.bin",
        "divaext_images.bin",
        "divaext_images.bin",
        "diva2nd#_images.bin",
        "diva2nd_images.bin",
};

SceUID datafd;
extern char filepath[256];

static SceUID block_id;
u32 cpk_count = 0;
cpknode *cpk_table;
char *image_filenames;
const char *cpk_file;

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

/**
 * load an image index table from the storage media
 * @param file_index - the filename array index to be used
 */
int load_image_index(int file_index) {
    u32 index_size;
    void *block_addr;

    SceUID fd = open_file(image_files[file_index]);
    if(fd < 0) {
        return 0;
    }

    SceSize size = (SceSize)sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);

    kprintf("image index size: %i bytes\n", size);
    block_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_KERNEL, "diva_img", PSP_SMEM_High, size, NULL);
    if(block_id < 0) {
        kprintf("cannot allocate memory block\n");
        return 0;
    }
    block_addr = sceKernelGetBlockHeadAddr(block_id);
    kprintf("allocated memory block: %08X\n", (u32)block_addr);

    sceIoRead(fd, block_addr, size);
    sceIoClose(fd);

    cpk_count = *(u32 *)block_addr;
    kprintf("image entries: %i\n", cpk_count);
    cpk_table = (cpknode *)((u32)block_addr + 4);
    kprintf("cpk table addr: %08X\n", (u32)cpk_table);

    index_size = (cpk_count * 8) + 4;
    index_size += 16 - (index_size % 16);
    image_filenames = (char *)block_addr + index_size;
    kprintf("image filenames start: %08X\n", (u32)image_filenames);

    // save the correct cpk container to use
    cpk_file = cpk_pack[file_index];
    return 1;
}

SceUID diva_open(const char *file, int flags, SceMode mode) {
    SceUID fd = sceIoOpen(file, flags, mode);
    if (fd >= 0) {
        // if the opened file is our cpk then lets save the file descriptor so
        // we can recognize it later
        if (strcmp(file, cpk_file) == 0) {
            kprintf("%s opened\n", cpk_file);
            datafd = fd;
        } else {
            kprintf("opened: %s, fd: %08X\n", file, fd);
        }
    }
    return fd;
}

int diva_read(SceUID fd, void *data, SceSize size) {
    u32 file_offset;
    cpknode *file_index;
    SceUID modfd;
    int res;
    u32 k1;

    if (fd == datafd) {
        // get the current file pointer
        file_offset = (u32)sceIoLseek(fd, 0, PSP_SEEK_CUR);
        // and find if the file part that the game is trying to read is in our list
        // of files to replace
        file_index = search_exact(file_offset, cpk_table, cpk_count);
        if(file_index != NULL) {
            kprintf("index found: %08X\n", *(u32 *)file_index);
            k1 = pspSdkSetK1(0);
            modfd = open_file(image_filenames + file_index->filename_offset);
            if(modfd >= 0) {
                kprintf("reading offset %08X, size: %08X\n", file_offset, size);
                // read our modified file instead, fooling the game
                res = sceIoRead(modfd, data, size);
                sceIoClose(modfd);
                kprintf("read returned %08X\n", res);
                // make sure to return the read bytes that the game is expecting
                res = (int)size;
                // and advance the file pointer so we fully simulated that the read from the
                // cpk has been done
                sceIoLseek(fd, size, PSP_SEEK_CUR);
                pspSdkSetK1(k1);
                return res;
            }
            pspSdkSetK1(k1);
        }
    }
    kprintf("reading %i bytes from fd: %08X, offset: %08X\n", size, fd, (u32)sceIoLseek(fd, 0, PSP_SEEK_CUR));
    return sceIoRead(fd, data, size);
}

int diva_close(SceUID fd) {
    if (fd == datafd) {
        // clear our file descriptor
        datafd = -1;
        kprintf("%s closed\n", cpk_file);
    } else {
        kprintf("closed fd: %08X\n", fd);
    }
    return sceIoClose(fd);
}
