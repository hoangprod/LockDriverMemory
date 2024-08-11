#pragma once


namespace G
{
    inline const char* TargetDriver = "nvlddmkm.sys";

    inline PKLDR_DATA_TABLE_ENTRY DriverEntry = 0;

    inline UNICODE_STRING RegistryPath = { 0 };
    inline UNICODE_STRING DriverPath = { 0 };

    inline UNICODE_STRING DeviceName = { 0 };
    inline UNICODE_STRING SymbolicLinkName = { 0 };


    inline PDRIVER_OBJECT DriverObject = NULL;
    inline PDEVICE_OBJECT DeviceObject = NULL;

    inline UINT_PTR	DriverImageBase;
    inline ULONG	DriverImageSize;

    inline UINT_PTR TargetDriverBase;
    inline ULONG    TargetDriverSize;
}