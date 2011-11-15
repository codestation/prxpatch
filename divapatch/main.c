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
#include <string.h>
#include "pspdefs.h"
#include "hook.h"
#include "reader.h"
#include "logger.h"

PSP_MODULE_INFO("divapatch", PSP_MODULE_KERNEL, 1, 0);
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
        "NPJH90226", // Project Diva Extend TGS Demo 1
        "NPJH90227", // Project Diva Extend Demo 2
        "ULJM05933", // Project Diva Extend
		"NPJH50465", // Project Diva Extend
};

const char *trans_files[] = {
        "diva1st_translation.bin",
        "diva2nd_translation.bin",
        "divaext_demo1.bin",
        "divaext_demo2.bin",
        "divaext_translation.bin",
        "divaext_translation.bin",
};
const stub stubs[] = {
        { 0x109F50BC, diva_open  }, // sceIoOpen
        { 0x6A638D83, diva_read  }, // sceIoRead
        { 0x810C4BC3, diva_close }, // sceIoClose
};

char filepath[256];
static STMOD_HANDLER previous = NULL;
static SceUID block_id = -1;
static void *block_addr = NULL;
static int patch_index = -1;
SceUID sema = 0;

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

void clear_caches(void) {
    IcacheClearAll();
    DcacheWritebackAll();
}

void patch_eboot()  {
    u32 count;
    SceUID fd;
    SceOff size;
    SceSize index_size, table_size;

    strrchr(filepath, '/')[1] = 0;
    strcat(filepath, trans_files[patch_index]);
    fd = sceIoOpen(filepath, PSP_O_RDONLY, 0777);
    if(fd < 0) {
        return;
    }
    size = sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);
    sceIoRead(fd, &count, 4);
    index_size = (count * 8) + 4;
    index_size += 16 - (index_size % 16);
    table_size = (SceSize)size - index_size;
    block_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "divatrans", PSP_SMEM_High, table_size, NULL);
    if(block_id < 0) {
        return;
    }
    block_addr = sceKernelGetBlockHeadAddr(block_id);
    kprintf("allocated block at %08X\n", (u32)block_addr);
    kprintf("found %i strings to pach\n", count);
    for(u32 i = 0; i < count; i++) {
        struct addr_data data;
        sceIoRead(fd, &data, sizeof(data));
        void *final_addr = (void *)((u32)(block_addr) + data.offset);
        kprintf("using %08X as string address\n", (u32)final_addr);

        // do an extra patch step for branch loads
        u32 extra_patch = 0;
        if(((u32)data.addr & 0xF0000000) == 0xE0000000) {
            extra_patch = 1;
            data.addr = (void **)((u32)data.addr | (u32)0xF0000000);
        }

        if((u32)data.addr & 0xF0000000) {
            data.addr = (void **)(((u32)data.addr) & ~0xF0000000);
            data.addr = (void **)(((u32)data.addr) | 0x40000000);
            kprintf("%02i) patching opcodes around addr: %08X\n", i, (u32)data.addr);
            u16 *code_addr = (u16 *)data.addr;
            u16 addr1 = (u32)final_addr & 0xFFFF;
            u16 addr2 = (u16)((((u32)final_addr) >> 16) & 0xFFFF);
            if(addr1 & 0x8000) {
                addr2++;
            }
            int j = 0;
            u16 mips_reg = (*(code_addr + 1)  >> 5) & 0x1F;
            kprintf("using register number %i for lui search\n", mips_reg);
            while(j < 32) { //maximum backtrace to look for a lui instruction
                u16 lui = *(code_addr - 1 - (j*2));
                kprintf("> checking %08X for lui: (%04X), regnum: %i\n", (u32)(code_addr - 1 - (j*2)), lui, (lui & 0x1F));

                if((lui & 0x3C00) == 0x3C00 && (lui & 0x1F) == mips_reg) { // lui
                    kprintf("> patching lower addr: %08X with %04X\n", (u32)code_addr, addr1);
                    *code_addr = addr1;
                    code_addr -= (2 + (j*2));
                    kprintf("> patching upper addr: %08X with %04X\n", (u32)code_addr, addr2);
                    *code_addr = addr2;
                    break;
                } else {
                    j++;
                }
            }
            if(j >= 32) {
                kprintf("Backtrace failed, cannot find matching lui for %08X\n", (u32)data.addr);
            }

            if(extra_patch) {
                extra_patch = 0;
                kprintf("making extra patch\n");
                j = 0;
                while(j < 32) {
                    u16 lui = *(code_addr - 1 - (j*2));
                    kprintf("> checking %08X for lui: (%04X), regnum: %i\n", (u32)(code_addr - 1 - (j*2)), lui, (lui & 0x1F));
                    if ((lui & 0x3C00) == 0x3C00 && (lui & 0x1F) == mips_reg) { // lui
                        u16 *branch_addr =  code_addr - (3 + (j * 2));
                        u16 branch = *branch_addr;
                        kprintf("checking if %04X is a branch\n", branch);
                        if(branch & 0x1400) {
                            u16 addr3 = (u16)((int)*(branch_addr -1) << 2);
                            kprintf("branch offset: %04X\n", addr3);
                            u16 jump = (u16)((u32)data.addr - ((u32)(code_addr - 1 - (j*2)) - 2));
                            kprintf("supposed jump: %04X\n", jump);
                            if(jump == addr3) {
                                kprintf("> patching lower addr: %08X with %04X\n", (u32)code_addr, addr1);
                                *code_addr = addr1;
                                code_addr -= (2 + (j * 2));
                                kprintf("> patching upper addr: %08X with %04X\n", (u32)code_addr, addr2);
                                *code_addr = addr2;
                                break;
                            }
                        }
                    }
                    j++;
                }
            }
        } else {
            data.addr = (void **)(((u32)data.addr) | 0x40000000);
            kprintf("%02i) patching pointer at addr: %08X\n", i, (u32)data.addr);
            *data.addr = final_addr;
        }
    }
    sceIoLseek(fd, index_size, PSP_SEEK_SET);
    sceIoRead(fd, block_addr, table_size);
    sceIoClose(fd);
}

int get_gameid(char *gameid) {
    SceUID fd = sceIoOpen(GAMEID_DIR, PSP_O_RDONLY, 0777);
    if (fd >= 0) {
        sceIoRead(fd, gameid, 10);
        sceIoClose(fd);
        if (gameid[4] == '-') {
            memmove(gameid + 4, gameid + 5, 5);
        }
        gameid[9] = '\0';
    }
    return fd;
}

int module_start_handler(SceModule2 * module) {
    if((module->text_addr & 0x80000000) != 0x80000000 && strcmp(module->modname, GAME_MODULE) == 0) {
        // game found, but since all versions use the same module name we need to find out the
        // correct game.
        kprintf("Project Diva found, loaded at %08X\n", module->text_addr);
        char gameid[16];
        if(get_gameid(gameid) >= 0) {
            kprintf("GAMEID: %s\n", gameid);
            //FIXME: change the lower bound to zero if a patch for Project Diva 1st is made
            for(int i = 1; i < ITEMSOF(diva_id); i++) {
                if(strcmp(gameid, diva_id[i]) == 0) {
                    kprintf("found match: %i\n", i);
                    patch_index = i;
                    break;
                }
            }
            if(patch_index >= 0) {
                patch_eboot();
                clear_caches();
                sceKernelSignalSema(sema, 1);
            }
        }
    }
    return previous ? previous(module) : 0;
}

inline void patch_imports(SceModule *module) {
    for (u32 i = 0; i < ITEMSOF(stubs); i++) {
        if(hook_import_bynid(module, "IoFileMgrForUser", stubs[i].nid, stubs[i].func, 1) < 0) {
            kprintf("Failed to hook %08X\n", stubs[i].nid);
        }
    }
}

int thread_start(SceSize args, void *argp) {
    strcpy(filepath, argp);
    sema = sceKernelCreateSema("divapatch_wake", 0, 0, 1, NULL);
    previous = sctrlHENSetStartModuleHandler(module_start_handler);
    sceKernelWaitSema(sema, 1, NULL);
    if(patch_index >= 4) {
        SceModule *module = sceKernelFindModuleByName(GAME_MODULE);
        if(module) {
            patch_imports(module);
        }
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
