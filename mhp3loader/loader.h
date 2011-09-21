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

#ifndef LOADER_H_
#define LOADER_H_

#include <pspkernel.h>

/* Avoid that "Old Plugin Support (PSP-Go only)" screw us */
#define XOR_KEY 0xDEADC0DE
#define XORED_MEMORY_STICK 0xE49DB3B3
#define XORED_INTERNAL_STORAGE 0xE49DA6BB

typedef union _dpath {
    u32 *device;
    const char *path;
} dpath;

enum Location {
    MEMORY_STICK,
    INTERNAL_STORAGE,
    INVALID = -1,
};

#define SET_DEVICENAME(p, l) { \
    dpath __d; \
    __d.path = (p); \
    *__d.device = XOR_KEY; \
    *__d.device ^= (l) == MEMORY_STICK ? XORED_MEMORY_STICK : XORED_INTERNAL_STORAGE; \
}

#define ITEMSOF(arr) (sizeof(arr) / sizeof(0[arr]))

int load_mod_index();
int load_quest_index();
u32 *find_mod_index(u32 value);
SceUID load_mod_file(u32 number);
u32 get_mod_number(u32 *pos);
void unload_mod_index();
void quest_override(u32 mod_number);

#endif /* LOADER_H_ */
