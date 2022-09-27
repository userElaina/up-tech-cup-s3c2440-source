!if 0
Copyright (c) Microsoft Corporation.  All rights reserved.
!endif
!if 0
Use of this source code is subject to the terms of the Microsoft end-user
license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
If you did not accept the terms of the EULA, you are not authorized to use
this source code. For a copy of the EULA, please see the LICENSE.RTF on your
install media.
!endif
!if 0


Module Name:

    makefile.inc

!endif

stepldr_romimage:
!IF "$(NOLINK)" == ""
    romimage $(ROMIMAGE_FLAGS) stepldr.bib
    debug $(_PLATFORMROOT)\$(_TGTPLAT)\target\$(_TGTCPU)\$(WINCEDEBUG)\stepldr.nb0 < trimnb0.scr
    
!IF "$(WINCEREL)"=="1"
    copy $(_PLATFORMROOT)\$(_TGTPLAT)\target\$(_TGTCPU)\$(WINCEDEBUG)\stepldr.* $(_FLATRELEASEDIR)
!ENDIF
!ENDIF
