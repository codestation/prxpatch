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

#include <pspge.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <pspsysmem.h>
#include <pspthreadman.h>
#include "bootsplash.h"

void *vram_base = 0;

void init_display() {
    vram_base = (void*) (0x40000000 | (u32) sceGeEdramGetAddr());
    void *vram = vram_base;
    sceDisplaySetMode(0, 480, 272);
    sceDisplaySetFrameBuf(vram, 512, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
}

void clear_display(u32 color) {
    int x;
    u32 *vram = vram_base;
    for(x = 0; x < (512 * 272); x++)
        *vram++ = color;
}

void fadeout_display(unsigned int decrease_by, SceUInt delay) {
    int x;
    for(x = 0; x < 16; x++) {
        uint8_t *p;
        int i;
        int h;
        for(h = 271; h >= 0; h--) {
            p = vram_base + (h*512*4);
            for(i = 0; i < 480; i++) {
                p[i*4] = p[i*4] >= decrease_by ? p[i*4] - decrease_by : 0;
                p[(i*4) + 1] = p[(i*4)+1] >= decrease_by ? p[(i*4)+1] - decrease_by : 0;
                p[(i*4) + 2] = p[(i*4)+2] >= decrease_by ? p[(i*4)+2] - decrease_by : 0;
            }
        }
        sceKernelDelayThread(delay);
    }

}

int draw_image(const char *path) {
    init_display();
    clear_display(0);
    sceDisplayWaitVblankStart();
    void *frame_addr = 0;
    int bufferwidth, pixelformat;
    sceDisplayGetFrameBuf(&frame_addr, &bufferwidth, &pixelformat, PSP_DISPLAY_SETBUF_IMMEDIATE);
    if(frame_addr == NULL)
        return -1;
    unsigned int ptr = (unsigned int)frame_addr;
    ptr |= ptr & 0x80000000 ?  0xA0000000 : 0x40000000;
    //void *frame_addr = vram_base;
    return write_bitmap((void *)ptr, PSP_DISPLAY_PIXEL_FORMAT_8888, path);
}

int write_bitmap(void *frame_addr, int format, const char *file) {
    SceUID fd = sceIoOpen(file,PSP_O_RDONLY, 0777);
    if(fd >= 0) {
        sceIoLseek32(fd, -391680, PSP_SEEK_END);
        SceUID memid = sceKernelAllocPartitionMemory(2, "boot_logo", PSP_SMEM_Low, 391680, NULL);
        if(memid < 0)
            return -1;
        void *pixel_data = sceKernelGetBlockHeadAddr(memid);
        sceIoRead(fd, pixel_data, 391680);
        switch(format) {
        /*case PSP_DISPLAY_PIXEL_FORMAT_565:
            read_565_data(frame_addr, pixel_data);
            break;
        case PSP_DISPLAY_PIXEL_FORMAT_5551:
            read_5551_data(frame_addr, pixel_data);
            break;
        case PSP_DISPLAY_PIXEL_FORMAT_4444:
            read_4444_data(frame_addr, pixel_data);
            break;*/
        case PSP_DISPLAY_PIXEL_FORMAT_8888:
            read_8888_data(frame_addr, pixel_data);
            break;
        }
        sceKernelFreePartitionMemory(memid);
        sceIoClose(fd);
        return 0;
    }
    return -1;
}

int read_8888_data(void *frame, void *pixel_data) {
    uint8_t *line;
    uint8_t *p;
    int i;
    int h;
    line = pixel_data;
    for(h = 271; h >= 0; h--) {
        p = frame + (h*512*4);
        for(i = 0; i < 480; i++) {
            p[i*4] = line[(i*3) + 2];
            p[(i*4) + 1] = line[(i*3) + 1];
            p[(i*4) + 2] = line[(i*3) + 0];
        }
        line += 480 * 3;
    }
    return 0;
}
/*
int read_5551_data(void *frame, void *pixel_data)
{
    uint8_t *line;
    uint16_t *p;
    int i;
    int h;

    line = pixel_data;
    for(h = 271; h >= 0; h--)
    {
        p = frame;
        p += (h * 512);
        for(i = 0; i < 480; i++)
        {
            p[i] = (line[(i*3) + 2] >> 3) & 0x1F;
            p[i] |= ((line[(i*3) + 1] >> 3) & 0x1F) << 5;
            p[i] |= ((line[(i*3) + 1] >> 3) & 0x1F) << 10;
        }
        line += 480*3;
    }

    return 0;
}

int read_565_data(void *frame, void *pixel_data)
{
    uint8_t *line;
    uint16_t *p;
    int i;
    int h;

    line = pixel_data;
    for(h = 271; h >= 0; h--)
    {
        p = frame;
        p += (h * 512);
        for(i = 0; i < 480; i++)
        {
            p[i] = (line[(i*3) + 2] >> 3) & 0x1F;
            p[i] |= ((line[(i*3) + 1] >> 2) & 0x3F) << 5;
            p[i] |= ((line[(i*3) + 1] >> 3) & 0x1F) << 11;
        }
        line += 480*3;
    }

    return 0;
}

int read_4444_data(void *frame, void *pixel_data)
{
    uint8_t *line;
    uint16_t *p;
    int i;
    int h;

    line = pixel_data;
    for(h = 271; h >= 0; h--)
    {
        p = frame;
        p += (h * 512);
        for(i = 0; i < 480; i++)
        {
            p[i] = (line[(i*3) + 2] >> 4) & 0xF;
            p[i] |= ((line[(i*3) + 1] >> 4) & 0xF) << 4;
            p[i] |= ((line[(i*3) + 1] >> 4) & 0xF) << 8;
        }
        line += 480*3;
    }

    return 0;
}
*/


