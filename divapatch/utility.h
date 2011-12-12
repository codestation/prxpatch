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

#ifndef UTILITY_H_
#define UTILITY_H_

#include <psputility.h>
#include <psputility_savedata.h>
#include <psputility_osk.h>

typedef struct SceUtilityScreenshotParam {
    pspUtilityDialogCommon base;
    char data[880];
} SceUtilityScreenshotParam;

int diva_save(SceUtilitySavedataParam * params);
int diva_osk(SceUtilityOskParams* params);
int diva_net(pspUtilityNetconfData *data);
int diva_shot(SceUtilityScreenshotParam *params);

#endif /* UTILITY_H_ */
