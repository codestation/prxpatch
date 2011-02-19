/*
 *  MHP3patch user module
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

#include "dialog.h"
#include "logger.h"

int netconf(pspUtilityNetconfData *data) {
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &data->base.language);
    return sceUtilityNetconfInitStart(data);
}

int osk(SceUtilityOskParams* params) {
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &params->base.language);
    params->data->language = PSP_UTILITY_OSK_LANGUAGE_DEFAULT;
    params->data->inputtype = 0xDF0F;
    return sceUtilityOskInitStart(params);
}

int msgdialog(pspUtilityMsgDialogParams *params) {
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &params->base.language);
    return sceUtilityMsgDialogInitStart(params);
}

int setsystemparam(int id, int value) {
    if(id == PSP_SYSTEMPARAM_ID_INT_LANGUAGE)
        return 0;
    return sceUtilitySetSystemParamInt(id, value);
}
