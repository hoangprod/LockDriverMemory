#include "Header/pch.h"
#include "Header/Helper.h"

size_t pPmldListCount = 0;
PMDL   pPmldList[100] = {};

NTSTATUS Helper::LockProcessNonDiscardableSections(PVOID TargetDriverBase)
{
    if (!TargetDriverBase)
    {
		DBE("Invalid driver base address");
		return STATUS_INVALID_PARAMETER;
	}

    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(TargetDriverBase);

    // Check if the DOS header is valid
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        DBE("Invalid DOS header signature");
        return STATUS_INVALID_PARAMETER;
    }

    PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>((reinterpret_cast<BYTE*>(TargetDriverBase) + dosHeader->e_lfanew));

    // Check if the NT headers signature is valid
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        DBE("Invalid NT header signature");
        return STATUS_INVALID_PARAMETER;
    }

    PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);

    // Iterate through each section
    for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i, ++sectionHeader)
    {
        // Skip discardable sections
        if (sectionHeader->Characteristics & IMAGE_SCN_MEM_DISCARDABLE)
        {
            DBP("Skipping discardable section: %s", sectionHeader->Name);
            continue;
        }

        // Calculate section's memory address and size
        PVOID sectionAddress = reinterpret_cast<PVOID>(reinterpret_cast<BYTE*>(TargetDriverBase) + sectionHeader->VirtualAddress);
        ULONG sectionSize = sectionHeader->Misc.VirtualSize;

        bool success = success = Helper::LockMemory(sectionAddress, sectionSize);

        if (!success)
        {
            DBE("Failed to lock section: %s", sectionHeader->Name);
            return STATUS_UNSUCCESSFUL;
        }
    }

    return STATUS_SUCCESS;
}


BOOLEAN Helper::LockMemory(PVOID Address, ULONG Size)
{
    PMDL mdl = IoAllocateMdl(Address, Size, FALSE, FALSE, NULL);

    if (mdl == NULL)
    {
        DBE("Failed to allocate MDL for address (%p) and size (%lx)", Address, Size);
        return FALSE;
    }

    __try
    {
        MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        IoFreeMdl(mdl);
        return FALSE;
    }

    pPmldList[pPmldListCount++] = mdl;

    DBP("Memory locked at address (%p) and size (%lx)", Address, Size);

    return TRUE;
}

BOOLEAN Helper::UnlockMemory(PMDL mdl)
{
    if (mdl == NULL)
    {
        DBE("Invalid Mdl");
        return FALSE;
    }

    __try
    {
        MmUnlockPages(mdl);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        IoFreeMdl(mdl);
        return FALSE;
    }

    IoFreeMdl(mdl);

    DBP("Memory unlocked");

    return TRUE;
}

NTSTATUS Helper::GetSystemModuleInformation(const char* ModuleName, UINT_PTR& OutBase, ULONG& OutSize)
{
    PSYSTEM_MODULE_ENTRY pSystemModuleInfo = 0;
    ULONG    size = 0x1000;
    NTSTATUS status = STATUS_SUCCESS;

    do {
        pSystemModuleInfo = static_cast<PSYSTEM_MODULE_ENTRY>(ExAllocatePool2(POOL_FLAG_NON_PAGED, size, 0x42414039));

        if (!pSystemModuleInfo) {
            DBE("Could not allocate PSYSTEM_MODULE_ENTRY struct due to memory issue.");
            return STATUS_NO_MEMORY;
        }
        status = ZwQuerySystemInformation(SystemModuleInformation, pSystemModuleInfo, size, &size);

        // If successfully allocate the pool, we can bail here and query the actual data
        //
        if (NT_SUCCESS(status))
        {
            break;
        }

        ExFreePool(pSystemModuleInfo);
        pSystemModuleInfo = 0;

        // Double the size for next time
        size *= 2;

    } while (status == STATUS_INFO_LENGTH_MISMATCH);

    if (pSystemModuleInfo)
    {
        PSYSTEM_MODULE_ENTRY pMod = ((PSYSTEM_MODULE_INFORMATION)pSystemModuleInfo)->Module;
        for (ULONG i = 0; i < ((PSYSTEM_MODULE_INFORMATION)pSystemModuleInfo)->Count; i++)
        {
            if (strstr((const char*)pMod[i].FullPathName, ModuleName))
            {
                OutBase = (UINT_PTR)pMod[i].ImageBase;
                OutSize = (ULONG)pMod[i].ImageSize;
                break;
            }
        }
    }

    if (pSystemModuleInfo)
    {
        ExFreePool(pSystemModuleInfo);
        pSystemModuleInfo = 0;
    }

    if (OutBase && OutSize)
        return STATUS_SUCCESS;

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS Helper::LockDriverMemory()
{
    // Find target driver
    if (!NT_SUCCESS(Helper::GetSystemModuleInformation(G::TargetDriver, G::TargetDriverBase, G::TargetDriverSize)))
    {
        DBE("Failed to get Target driver base");
        return STATUS_UNSUCCESSFUL;
    }

    // Lock the non-discardable sections of the driver
    NTSTATUS status = LockProcessNonDiscardableSections(reinterpret_cast<PVOID>(G::TargetDriverBase));
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    DBP("Target driver memory locked");
    return STATUS_SUCCESS;
}

void Helper::UnlockDriverMemory()
{
    for (size_t i = 0; i < pPmldListCount; ++i)
    {
        if (pPmldList[i] && !UnlockMemory(pPmldList[i]))
        {
			DBE("Failed to unlock memory at index (%lld)", i);
            continue;
		}
	}

    // Reset this but we have no plan on re-using this again because we should be unloading driver right now
    //
    pPmldListCount = 0;
}