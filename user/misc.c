/*
 * misc.c
 *
 *  Created on: 12/02/2011
 *      Author: code
 */

#include <pspiofilemgr.h>
#include "misc.h"

SceUID transfd = -1;
int reopen = 1;

void reopen_translation() {
    if(reopen) {
        transfd = sceIoOpen((model == MODEL_PSPGO) ? TRANSLATION_PATH_GO : TRANSLATION_PATH_MS, PSP_O_RDONLY, 0777);
        reopen = 0;
    }
}
