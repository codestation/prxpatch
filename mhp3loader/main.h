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

#ifndef MAIN_H_
#define MAIN_H_

#include <psploadcore.h>

#define ITEMSOF(arr) (sizeof(arr) / sizeof(0[arr]))
#define UNUSED __attribute__((unused))

typedef int (*STMOD_HANDLER)(SceModule *);

typedef struct {
    u32 nid;
    void *func;
} stub;

STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler);

#endif /* MAIN_H_ */
