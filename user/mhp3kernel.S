    .set noreorder

#include "pspstub.s"

    STUB_START "mhp3kernel",0x40090011,0x00010005
    STUB_FUNC  0xA37D9989,registerfunctions
    STUB_END
