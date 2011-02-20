/*
 * misc.c
 *
 *  Created on: 12/02/2011
 *      Author: code
 */

#include <pspiofilemgr.h>
#include "misc.h"
#include "logger.h"

volatile SceUID transfd = -1;
volatile int reopen = 1;

int reopen_translation() {
    if(reopen) {
        transfd = sceIoOpen((model == MODEL_PSPGO) ? TRANSLATION_PATH_GO : TRANSLATION_PATH_MS, PSP_O_RDONLY, 0777);
        if(transfd >= 0)
            reopen = 0;
    }
    return reopen;
}
