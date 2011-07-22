/*
 * Copyright 2010 by Coldbird
 *
 * libs.h - Import Hooking Functions
 *
 * This file is part of Adhoc Emulator.
 *
 * Adhoc Emulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Adhoc Emulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Adhoc Emulator.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LIBS_H_
#define _LIBS_H_

#include <psploadcore.h>

//query syscall number
unsigned int sceKernelQuerySystemCall(void * function);

//hook function via import (can do jump & syscall hooks)
int hook_import_bynid(SceModule * module, const char * library, unsigned int nid, void * function, int syscall);

#endif
