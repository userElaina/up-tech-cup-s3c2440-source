@REM
@REM Copyright (c) Microsoft Corporation.  All rights reserved.
@REM
@REM
@REM Use of this sample source code is subject to the terms of the Microsoft
@REM license agreement under which you licensed this sample source code. If
@REM you did not accept the terms of the license agreement, you are not
@REM authorized to use this sample source code. For the terms of the license,
@REM please see the license agreement between you and Microsoft or, if applicable,
@REM see the LICENSE.RTF on your install media or the root of your tools installation.
@REM THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
@REM
echo Tencent ProjSysgen.bat executing.
if /i not "%1"=="preproc" goto Not_Preproc
    set TENCENT_MODULES=
    goto :EOF
:Not_Preproc
if /i not "%1"=="pass1" goto Not_Pass1

if "%SYSGEN_TENCENT_MOBILEQQ%"=="" goto no_tencent_qq
ECHO Setting SYSGEN VARIABLES IN Tencent ProjSysgen.bat
	set SYSGEN_ATL=1
	set SYSGEN_CELLCORE=1
	set SYSGEN_CELLCORE_SMS=1
	set SYSGEN_CELLCORE_WAP=1
	set SYSGEN_TCPIP=1
	set SYSGEN_TAPI=1
	set SYSGEN_IMAGING=1
	set SYSGEN_IMAGING_PNG_DECODE=1
    set TENCENT_MODULES=%TENCENT_MODULES% qq
:no_tencent_qq

    goto EOF
:Not_Pass1
if /i not "%1"=="pass2" goto Not_Pass2
    goto :EOF
:Not_Pass2
if /i not "%1"=="report" goto Not_Report
    if not "%TENCENT_MODULES%"==""     echo TENCENT_MODULES=%TENCENT_MODULES%
    goto :EOF
:Not_Report
echo %0 Invalid parameter %1
:EOF
