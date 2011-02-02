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

#include <pspsdk.h>
#include "imports.h"
#include "sceio.h"
#include "logger.h"

// PSP model
int model;

PSP_MODULE_INFO("mhp3patch_user", PSP_MODULE_USER, 1, 0);
PSP_HEAP_SIZE_KB(0);

int module_start(SceSize args, void * argp) {
    void *functions[4];
    functions[0] = open;
    functions[1] = read;
    functions[2] = close;
    functions[3] = callback;
    model = registerfunctions(functions);
    return 0;
}

int module_stop(SceSize args, void * argp) {
    return 0;
}
