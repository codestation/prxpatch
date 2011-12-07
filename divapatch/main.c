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

#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <pspimpose_driver.h>
#include <string.h>
#include "pspdefs.h"
#include "hook.h"
#include "reader.h"
#include "utility.h"
#include "logger.h"

#define PLUGIN_NAME "divapatch"

PSP_MODULE_INFO(PLUGIN_NAME, PSP_MODULE_KERNEL, 1, 0);
PSP_HEAP_SIZE_KB(0);

#define GAME_MODULE "PdvApp"
#define GAMEID_DIR "disc0:/UMD_DATA.BIN"

#define ITEMSOF(arr) (int)(sizeof(arr) / sizeof(0[arr]))

struct addr_data {
    void **addr;
    u32 offset;
}__attribute__((packed));

typedef struct {
    u32 nid;
    void *func;
} stub;

const char *diva_id[] = {
        "ULJM05472", // Project Diva
        "ULJM05681", // Project Diva 2nd
        "ULJM05933", // Project Diva Extend
		"NPJH50465", // Project Diva Extend
};

const char *trans_files[] = {
        "diva1st_translation.bin",
        "diva2nd_translation.bin",
        "divaext_translation.bin",
        "divaext_translation.bin",
};

const char *embedded_files[] = {
        "diva1st_embedded.bin",
        "diva2nd_embedded.bin",
        "divaext_embedded.bin",
        "divaext_embedded.bin",
};

const stub stubs[] = {
        { 0x109F50BC, diva_open  }, // sceIoOpen
        { 0x6A638D83, diva_read  }, // sceIoRead
        { 0x810C4BC3, diva_close }, // sceIoClose
};

const stub utility_stubs[] = {
        { 0x50C4CD57, diva_save  }, // sceUtilitySavedataInitStart
        { 0xF6269B82, diva_osk   }, // sceUtilityOskInitStart
        { 0x4DB1E739, diva_net  }, // sceUtilityNetconfInitStart
};

char filepath[256];

static SceUID block_id = -1;
static SceUID table_id = -1;
static int patch_index = -1;

static void *block_addr = NULL;
static void *table_addr = NULL;

static STMOD_HANDLER previous = NULL;

static SceUID sema = 0;

inline void DcacheWritebackAll(void) {
    __asm__ volatile("\
    .word 0x40088000; .word 0x24090800; .word 0x7D081180;\
    .word 0x01094804; .word 0x00004021; .word 0xBD140000;\
    .word 0xBD140000; .word 0x25080040; .word 0x1509FFFC;\
    .word 0x00000000; .word 0x0000000F; .word 0x00000000;\
    "::);
}

inline void IcacheClearAll(void) {
    __asm__ volatile("\
    .word 0x40088000; .word 0x24091000; .word 0x7D081240;\
    .word 0x01094804; .word 0x4080E000; .word 0x4080E800;\
    .word 0x00004021; .word 0xBD010000; .word 0xBD030000;\
    .word 0x25080040; .word 0x1509FFFC; .word 0x00000000;\
    "::);
}

/**
 * clears the instruction cache
 */
void clear_caches(void) {
    IcacheClearAll();
    DcacheWritebackAll();
}

void patch_embedded() {
    u32 count;
    SceUID fd;
    SceSize size;
    SceSize index_size, table_size;
    SceUID embedded_id;
    void *embedded_addr;

    // use the plugin directory to load the patchfile from there
    strrchr(filepath, '/')[1] = 0;
    strcat(filepath, embedded_files[patch_index]);

    fd = sceIoOpen(filepath, PSP_O_RDONLY, 0777);
    if(fd < 0) {
        return;
    }

    // get the translation file size
    size = (SceSize)sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);

    embedded_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_KERNEL, "divatable", PSP_SMEM_Low, size, NULL);
    if(embedded_id < 0) {
        return;
    }
    embedded_addr = sceKernelGetBlockHeadAddr(embedded_id);
    kprintf("allocated table block at %08X\n", (u32)embedded_addr);

    // read the whole file
    sceIoLseek(fd, 0, PSP_SEEK_SET);
    sceIoRead(fd, embedded_addr, size);
    sceIoClose(fd);

    // get the number of entries in the translation file
    count = *(u32 *)embedded_addr;

    // calculate the index table size
    index_size = (count * 8) + 4;
    // and the padding (if any)
    index_size += 16 - (index_size % 16);
    // calculate the required size to hold all the translated strings (without the index table)
    table_size = size - index_size;

    kprintf("found %i strings to patch\n", count);

    for(u32 i = 0; i < count; i++) {
        struct addr_data *data;

        // get the current table pointer
        data = &((struct addr_data *)((char *)embedded_addr + 4))[i];

        // and calculate the address that is gonna have our string in memory
        void *final_addr = (void *)((u32)((char *)embedded_addr + index_size) + data->offset);
        //kprintf("using %08X as string address\n", (u32)final_addr);

        // skip null strings
        if(strcmp(final_addr, "NULL") != 0) {
            strcpy((char *)data->addr, final_addr);
        }
    }
    sceKernelFreePartitionMemory(embedded_id);
}

void patch_eboot()  {
    u32 count;
    SceUID fd;
    SceOff size;
    SceSize index_size, table_size;

    // use the plugin directory to load the patchfile from there
    strrchr(filepath, '/')[1] = 0;
    strcat(filepath, trans_files[patch_index]);

    fd = sceIoOpen(filepath, PSP_O_RDONLY, 0777);
    if(fd < 0) {
        return;
    }

    // get the translation file size
    size = sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);

    // get the number of entries in the translation file
    sceIoRead(fd, &count, 4);

    // calculate the index table size
    index_size = (count * 8) + 4;
    // and the padding (if any)
    index_size += 16 - (index_size % 16);
    // calculate the required size to hold all the translated strings (without the index table)
    table_size = (SceSize)size - index_size;

    // allocate the translated string block
    block_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "divatrans", PSP_SMEM_High, table_size, NULL);
    if(block_id < 0) {
        return;
    }
    block_addr = sceKernelGetBlockHeadAddr(block_id);
    kprintf("allocated block at %08X\n", (u32)block_addr);

    // read the translated strings
    sceIoLseek(fd, index_size, PSP_SEEK_SET);
    sceIoRead(fd, block_addr, table_size);

    // allocate the index table
    table_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_KERNEL, "divatable", PSP_SMEM_Low, count * 8, NULL);
    if(table_id < 0) {
        return;
    }
    table_addr = sceKernelGetBlockHeadAddr(table_id);
    kprintf("allocated table block at %08X\n", (u32)table_addr);

    // read the translated strings
    sceIoLseek(fd, 4, PSP_SEEK_SET);
    sceIoRead(fd, table_addr, count * 8);

    sceIoClose(fd);

    kprintf("found %i strings to patch\n", count);

    // lets process all the strings in the table
    for(u32 i = 0; i < count; i++) {
        struct addr_data *data;

        // get the current table pointer
        data = &((struct addr_data *)table_addr)[i];

        // and calculate the address that is gonna have our string in memory
        void *final_addr = (void *)((u32)(block_addr) + data->offset);
        //kprintf("using %08X as string address\n", (u32)final_addr);

        int half_patch = 0;

        // if the address was marked as "^x08XXXXXX" in the source file then lets
        // only make a lui patch with the upper address
        if(((u32)data->addr & 0xF0000000) == 0xE0000000) {
            half_patch = 1;
            // mark the last byte as "F" so it can pass the next "if" conditional
            data->addr = (void **)((u32)data->addr | (u32)0xF0000000);

        // if the address was marked as "$x08XXXXXX" in the source file then lets
        // only make patch with the lower address
        } else if(((u32)data->addr & 0xF0000000) == 0xD0000000) {
            half_patch = -1;
            // mark the last byte as "F" so it can pass the next "if" conditional
            data->addr = (void **)((u32)data->addr | (u32)0xF0000000);
        }

        // if the address was marked as "!x08000000" in the source file then lets
        // patch the lower address in the current instruction and the upper address
        // in the lui instruction above it
        if((u32)data->addr & 0xF0000000) {
            // clear our "F" marker
            data->addr = (void **)(((u32)data->addr) & ~0xF0000000);
            // and use "4" instead (0x4XXXXXXX == uncached access, not sure if this is necessary)
            data->addr = (void **)(((u32)data->addr) | 0x40000000);

            u16 *code_addr = (u16 *)data->addr;

            // split or string address into lower/upper components
            u16 addr1 = (u32)final_addr & 0xFFFF;
            u16 addr2 = (u16)((((u32)final_addr) >> 16) & 0xFFFF);

            // increase by one if the lower address part if more than 0x7FFF
            // (this is required when changing the address of an instruction)
            if(addr1 & 0x8000) {
                addr2++;
            }

            // if the patch was an upper-address only then patch the lui instruction
            // and continue
            if(half_patch > 0) {
                half_patch = 0;
                //kprintf("%02i) patching upper offset at addr: %08X with %04X\n", i, (u32)data.addr, addr2);
                *code_addr = addr2;
                continue;
            } else if(half_patch < 0) {
                half_patch = 0;
                //kprintf("%02i) patching lower offset at addr: %08X with %04X\n", i, (u32)data.addr, addr2);
                *code_addr = addr1;
                continue;
            }

            //kprintf("%02i) patching opcodes around addr: %08X\n", i, (u32)data.addr);

            // get the mips register number used in the instruction. That number is located at the
            // bit 16-21. Don't forget the endianess so we have to advance 2 bytes to position our
            // pointer corrently and do the extraction. We use this number in the backtracking phrase
            // so we don't patch the incorrect lui instrcution.
            u16 mips_reg = (*(code_addr + 1)  >> 5) & 0x1F;
            //kprintf("using register number %i for lui search\n", mips_reg);

            int j = 0;
            //maximum backtrace to look for a lui instruction, lets check the above 32 instructions
            while(j < 32) {

                // extract out supposed lui instruction
                u16 lui = *(code_addr - 1 - (j*2));
                //kprintf("> checking %08X for lui: (%04X), regnum: %i\n", (u32)(code_addr - 1 - (j*2)), lui, (lui & 0x1F));

                // lets check if we have the lui instruction (0x3C) and it uses rhe same register number
                // than the instruction used in the lower address calculation
                if((lui & 0x3C00) == 0x3C00 && (lui & 0x1F) == mips_reg) { // lui
                    //kprintf("> patching lower addr: %08X with %04X\n", (u32)code_addr, addr1);
                    *code_addr = addr1;
                    // advance the pointer to our matching lui and patch its upper address
                    code_addr -= (2 + (j*2));
                    //kprintf("> patching upper addr: %08X with %04X\n", (u32)code_addr, addr2);
                    *code_addr = addr2;
                    break;
                } else {
                    j++;
                }
            }
            if(j >= 32) {
                kprintf("Backtrace failed, cannot find matching lui for %08X\n", (u32)data->addr);
            }
        } else {
            // the address to patch is in clear view so we don't have to do all crap above and
            // just overwrite the address
            data->addr = (void **)(((u32)data->addr) | 0x40000000);
            //kprintf("%02i) patching pointer at addr: %08X\n", i, (u32)data.addr);
            *data->addr = final_addr;
        }
    }
    sceKernelFreePartitionMemory(table_id);
}

/**
 * Gets our gameid from the UMD disk (there is a function who returns that but sadly
 * not all CFW resolves that function NID).
 */
int get_gameid(char *gameid) {
    SceUID fd = sceIoOpen(GAMEID_DIR, PSP_O_RDONLY, 0777);
    if (fd >= 0) {
        sceIoRead(fd, gameid, 10);
        sceIoClose(fd);
        // remove the "-" from the gameid
        if (gameid[4] == '-') {
            memmove(gameid + 4, gameid + 5, 5);
        }
        gameid[9] = '\0';
    }
    return fd;
}

int module_start_handler(SceModule2 * module) {
    // lets execute the previous start handlers first to give a chance
    // to nploader to complete its hooking phrase.
    int ret = previous ? previous(module) : 0;

    // tests if the module is usermode (skip all the kernel modules)
    if((module->text_addr & 0x80000000) != 0x80000000 && strcmp(module->modname, GAME_MODULE) == 0) {
        // game found, but since all versions use the same module name we need
        // to find out the correct game.
        kprintf("Project Diva found, loaded at %08X\n", module->text_addr);
        char gameid[16];

        if(get_gameid(gameid) >= 0) {
            kprintf("GAMEID: %s\n", gameid);
            //FIXME: change the "for" lower bound to zero if a patch for Project Diva 1st is made
            for(int i = 1; i < ITEMSOF(diva_id); i++) {
                if(strcmp(gameid, diva_id[i]) == 0) {
                    kprintf("found match: %i\n", i);
                    patch_index = i;
                    break;
                }
            }
            // valid gameid
            if(patch_index >= 0) {
                patch_embedded();
                patch_eboot();
                clear_caches();
                // wake up our main thread so it can hook the file i/o funs of the game
                sceKernelSignalSema(sema, 1);
            }
        }
    }
    return ret;
}

/**
 * Patch the imports of the game eboot
 * @param module: module structure to have it hooked
 */
void patch_imports(SceModule *module) {
    for (u32 i = 0; i < ITEMSOF(stubs); i++) {
        if(hook_import_bynid(module, "IoFileMgrForUser", stubs[i].nid, stubs[i].func, 1) < 0) {
            kprintf("Failed to hook %08X\n", stubs[i].nid);
        }
    }
}

/**
 * Patch the imports of the game eboot (osk/net/save dialogs)
 * @param module: module structure to have it hooked
 */
void patch_utility(SceModule *module) {
    for (u32 i = 0; i < ITEMSOF(stubs); i++) {
        if(hook_import_bynid(module, "sceUtility", utility_stubs[i].nid, utility_stubs[i].func, 1) < 0) {
            kprintf("Failed to hook %08X\n", utility_stubs[i].nid);
        }
    }
}

void change_lang(int lang) {
    int button;

    sceImposeGetLanguageMode((int[]){0}, &button);
    sceImposeSetLanguageMode(lang, button);
}

int thread_start(SceSize args, void *argp) {
    // save the plugin location
    strcpy(filepath, argp);

    // create a semaphore to know when the eboot patch is done
    sema = sceKernelCreateSema("divapatch_wake", 0, 0, 1, NULL);

    previous = sctrlHENSetStartModuleHandler(module_start_handler);

    // wait here until the patch is done
    sceKernelWaitSema(sema, 1, NULL);

    // load the image index table
    if(patch_index >= 0) {
        // change the impose language
        change_lang(PSP_SYSTEMPARAM_LANGUAGE_ENGLISH);
        if(load_image_index(patch_index)) {
            SceModule *module = sceKernelFindModuleByName(GAME_MODULE);
            if (module) {
                // redirect the file open, read and close operations to our plugin
                kprintf("patching imports\n");
                patch_imports(module);

                kprintf("patching utility funcs\n");
                patch_utility(module);
            }
            // 0x333A34AE == nploader's np_open NID
            u32 func = sctrlHENFindFunction("nploader", "nploader", 0x333A34AE);
            if(func) {
                kprintf("nploader found, redirecting sceIoOpen to np_open: %08X\n", func);
                module = sceKernelFindModuleByName(PLUGIN_NAME);
                // 0x109F50BC == sceIoOpen NID
                if(hook_import_bynid(module, "IoFileMgrForKernel", 0x109F50BC, (void *)func, 0) < 0) {
                    kprintf("Failed to hook %08X\n", 0x109F50BC);
                }
        }
        }
    } else {
        kprintf("the image index wasn't found\n");
    }
    sceKernelDeleteSema(sema);
	return 0;
}

int module_start(SceSize args, void *argp) {
    SceUID thid = sceKernelCreateThread("divapatch_main", thread_start, 0x22, 0x2000, 0, NULL);
    if(thid >= 0) {
        sceKernelStartThread(thid, args, argp);
    }
    return 0;
}

int module_stop(SceSize args, void *argp) {
    if(block_id >= 0) {
        sceKernelFreePartitionMemory(block_id);
    }
	return 0;
}
