#pragma once


namespace Helper
{

    NTSTATUS LockDriverMemory();
    VOID     UnlockDriverMemory();

    BOOLEAN  LockMemory(PVOID Address, ULONG Size);
    BOOLEAN  UnlockMemory(PMDL mdl);


    NTSTATUS GetSystemModuleInformation(const char* ModuleName, UINT_PTR& OutBase, ULONG& OutSize);
    NTSTATUS LockProcessNonDiscardableSections(PVOID TargetDriverBase);
}
