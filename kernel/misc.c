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

#include <pspiofilemgr.h>
#include "misc.h"
#include "logger.h"

volatile SceUID transfd = -1;
int model = -1;

int sceKernelGetModel();

void reopen_translation() {
    if(model < 0) {
        model = sceKernelGetModel();
    }
    sceIoClose(transfd);
    transfd = sceIoOpen((model == MODEL_PSPGO) ? TRANSLATION_PATH_GO : TRANSLATION_PATH_MS, PSP_O_RDONLY, 0777);
}
