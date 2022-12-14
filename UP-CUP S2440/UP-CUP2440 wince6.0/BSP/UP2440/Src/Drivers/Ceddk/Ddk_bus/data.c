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
#include <bsp.h>

//------------------------------------------------------------------------------
//
//  Function:  HalGetBusDataByOffset
//
//  This function read bus configuration data. It convert input parameters
//  to new format and then it calls kernel to do rest.
//
ULONG HalGetBusDataByOffset(
    BUS_DATA_TYPE busDataType, ULONG busNumber, ULONG slotNumber,
    VOID *pBuffer, ULONG offset, ULONG length
) {
    OAL_DDK_PARAMS params;
    PCI_SLOT_NUMBER slot;
    UINT32 outSize, rc = 0;

    params.function = IOCTL_OAL_READBUSDATA;
    params.rc = 0;
    switch (busDataType) {
    case PCIConfiguration:
        params.busData.devLoc.IfcType = PCIBus;
        slot.u.AsULONG = slotNumber;
        params.busData.devLoc.BusNumber = busNumber >> 8;
        params.busData.devLoc.LogicalLoc = (busNumber & 0xFF) << 16;
        params.busData.devLoc.LogicalLoc |= slot.u.bits.DeviceNumber << 8;
        params.busData.devLoc.LogicalLoc |= slot.u.bits.FunctionNumber;
        rc = TRUE;
        break;
    }
    params.busData.offset = offset;
    params.busData.length = length;
    params.busData.pBuffer = pBuffer;

    if (KernelIoControl(
        IOCTL_HAL_DDK_CALL, &params, sizeof(params), NULL, 0, &outSize
    )) {
        rc = params.rc;
    }

    return rc;
}

//------------------------------------------------------------------------------
//
// Function:     HalSetBusDataByOffset
//
//  This function write bus configuration data. It convert input parameters
//  to new format and then it calls kernel to do rest.
//
ULONG HalSetBusDataByOffset(
    BUS_DATA_TYPE busDataType, ULONG busNumber, ULONG slotNumber, VOID *pBuffer,
    ULONG offset, ULONG length
) {
    OAL_DDK_PARAMS params;
    PCI_SLOT_NUMBER slot;
    UINT32 outSize, rc = 0;

    params.function = IOCTL_OAL_WRITEBUSDATA;
    params.rc = 0;
    switch (busDataType) {
    case PCIConfiguration:
        params.busData.devLoc.IfcType = PCIBus;
        slot.u.AsULONG = slotNumber;
        params.busData.devLoc.BusNumber = busNumber >> 8;
        params.busData.devLoc.LogicalLoc = (busNumber & 0xFF) << 16;
        params.busData.devLoc.LogicalLoc |= slot.u.bits.DeviceNumber << 8;
        params.busData.devLoc.LogicalLoc |= slot.u.bits.FunctionNumber;
        rc = TRUE;
        break;
    }
    params.busData.offset = offset;
    params.busData.length = length;
    params.busData.pBuffer = pBuffer;

    if (KernelIoControl(
        IOCTL_HAL_DDK_CALL, &params, sizeof(params), NULL, 0, &outSize
    )) {
        rc = params.rc;
    }

    return rc;
}

//------------------------------------------------------------------------------
