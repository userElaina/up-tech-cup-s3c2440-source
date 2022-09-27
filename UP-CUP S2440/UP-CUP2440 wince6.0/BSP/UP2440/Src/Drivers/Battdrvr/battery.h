//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

// This file describes the stub battery interface.  It is only intended
// for use on systems without real batteries, for prototyping battery-
// sensitive software.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#pragma pack(4)
typedef struct _BatteryStatus_tag {
    SYSTEM_POWER_STATUS_EX2 sps;
    WORD    wMainLevels;
    WORD    wBackupLevels;
    BOOL    fSupportsChange;
    BOOL    fChanged;
} BATTERY_STATUS, *PBATTERY_STATUS;
#pragma pack()

#define BATTERY_STATUS_FILE TEXT("Battery File")
#define BATTERY_FILE_MUTEX  TEXT("Battery File Mutex")

#ifdef __cplusplus
}
#endif  /* __cplusplus */

