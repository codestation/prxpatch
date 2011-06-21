    .set noreorder

#include "pspstub.s"

    STUB_START "SysMemForKernel",0x00010011,0x00010005
    STUB_FUNC 0x458A70B5,sceKernelGetModel
    STUB_END

    STUB_START "SystemCtrlForKernel",0x00090000,0x00020005
    STUB_FUNC 0x1C90BECB,sctrlHENSetStartModuleHandler
    STUB_FUNC 0x159AF5CC,sctrlHENFindFunction
    STUB_END

    STUB_START "InterruptManagerForKernel",0x00010011,0x00010005
    STUB_FUNC 0x8B61808B,sceKernelQuerySystemCall
    STUB_END
