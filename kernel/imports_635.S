    .set noreorder

#include "pspstub.s"

    STUB_START "SysMemForKernel",0x00010011,0x00010005
    STUB_FUNC 0x3C4C5630,sceKernelGetUMDData
    STUB_FUNC 0x458A70B5,sceKernelGetModel
    STUB_END

    STUB_START "InterruptManagerForKernel",0x00010000,0x00010005
    STUB_FUNC 0x399FF74C,sceKernelQuerySystemCall
    STUB_END

    STUB_START "LoadCoreForKernel",0x00010000,0x00010005
    STUB_FUNC 0xEF8A0BEA,sceKernelFindModuleByName
    STUB_END
