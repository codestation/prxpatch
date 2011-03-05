/*
 * Copyright 2010 by Coldbird
 *
 * libs.c - Import Hooking Functions
 *
 * This file is part of Adhoc Emulator.
 *
 * Adhoc Emulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Adhoc Emulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Adhoc Emulator.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <psputils.h>
#include <string.h>
#include "libs.h"

typedef struct
{
  const char * name;
  unsigned short version;
  unsigned short attribute;
  unsigned char entLen;
  unsigned char varCount;
  unsigned short funcCount;
  unsigned int * fnids;
  unsigned int * funcs;
  unsigned int * vnids;
  unsigned int * vars;
}PspModuleImport;

PspModuleImport * find_import_lib(SceModule * module, const char * library)
{
  //library import pointer
  PspModuleImport * result = NULL;

  //valid arguments
  if(module && library)
  {
    //check stub table
    unsigned int x = 0; while(x < module->stub_size)
    {
      //grab library stub
      result = (PspModuleImport *)(module->stub_top + x);
      //library found
      if(result->name && strcmp(result->name, library) == 0)
      {
        //stop search
        break;
      }

      //move forward
      x += result->entLen * 4;
    }

    //library not found
    if(x == module->stub_size) result = NULL;
  }

  //return library import pointer
  return result;
}

unsigned int find_import_bynid(SceModule * module, const char * library, unsigned int nid)
{
  //stub address
  unsigned int result = 0;

  //find library import
  PspModuleImport * import = find_import_lib(module, library);

  //found import
  if(import)
  {
    //find stub
    unsigned int x = 0; for(; x < import->funcCount; x++)
    {
      //found stub
      if(import->fnids[x] == nid)
      {
        //save stub address
        result = (unsigned int)&import->funcs[x * 2];

        //stop search
        break;
      }
    }
  }

  //return stub address
  return result;
}

void api_hook_import(unsigned int address, void * function)
{
  //valid address
  if(address)
  {
    //asm jmp
    *(unsigned int *)(address) = 0x08000000 | ((unsigned int)function & 0x0FFFFFFF) >> 2;

    //asm nop
    *(unsigned int *)(address + 4) = 0;

    //flush cache
    sceKernelDcacheWritebackInvalidateRange((const void *)address, 8);
    sceKernelIcacheInvalidateRange((const void *)address, 8);
  }
}

void api_hook_import_syscall(unsigned int address, void * function)
{
  //valid address
  if(address)
  {
    //asm jr $ra
    *(unsigned int *)(address) = 0x03E00008;

    //asm syscall #
    *(unsigned int *)(address + 4) = (((sceKernelQuerySystemCall(function)) << 6) | 12);

    //flush cache
    sceKernelDcacheWritebackInvalidateRange((const void *)address, 8);
    sceKernelIcacheInvalidateRange((const void *)address, 8);
  }
}

int hook_import_bynid(SceModule * module, const char * library, unsigned int nid, void * function, int syscall)
{
  //result
  int result = 0;

  //find import stub
  unsigned int stub = find_import_bynid(module, library, nid);

  //found stub
  if(stub)
  {
    //write syscall
    if(syscall) api_hook_import_syscall(stub, function);

    //write jump
    else api_hook_import(stub, function);
  }

  //stub not found
  else result = -1;

  //return result
  return result;
}
