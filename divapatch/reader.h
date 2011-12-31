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

#ifndef SCEIO_H_
#define SCEIO_H_

#include <pspsdk.h>

typedef struct {
    u32 offset;
    u32 size;
    const char *filename;
} node;

typedef struct {
    u32 cpk_offset;
    u32 filename_offset;
} cpknode;

int load_image_index(int file_index);
SceUID diva_open(const char *file, int flags, SceMode mode);
int diva_read(SceUID fd, void *data, SceSize size);
int diva_aread(SceUID fd, void *data, SceSize size);
int diva_wait(SceUID fd, SceInt64 *res);
int diva_poll(SceUID fd, SceInt64 *res);
int diva_waitc(SceUID fd, SceInt64 *res);
int diva_close(SceUID fd);

#endif /* SCEIO_H_ */
