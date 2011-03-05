/*
 *  MHP3patch user module
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

#ifndef SCEIO_H_
#define SCEIO_H_

#include <pspsdk.h>

extern SceUID io_sema;
extern SceSize data_start;
extern unsigned int patch_count;
extern unsigned int patch_size[256];

int mhp3_callback(const char *name, SceKernelCallbackFunction func, void *arg);
SceUID mhp3_open(const char *file, int flags, SceMode mode);
int mhp3_read(SceUID fd, void *data, SceSize size);
SceOff mhp3_seek(SceUID fd, SceOff offset, int whence);
int mhp3_close(SceUID fd);

#endif /* SCEIO_H_ */
