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

int diva_callback(const char *name, SceKernelCallbackFunction func, void *arg);
SceUID diva_open(const char *file, int flags, SceMode mode);
int diva_read(SceUID fd, void *data, SceSize size);
SceOff diva_seek(SceUID fd, SceOff offset, int whence);
int diva_close(SceUID fd);

#endif /* SCEIO_H_ */
