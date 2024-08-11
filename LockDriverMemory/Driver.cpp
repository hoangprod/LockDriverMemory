#include "Header/pch.h"
#include "Header/Helper.h"

NTSTATUS DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	SEH_START();

	DBP("DriverUnload");

	Helper::UnlockDriverMemory();


	if (G::DeviceObject)
	{
		IoDeleteSymbolicLink(&G::SymbolicLinkName);
		IoDeleteDevice(G::DeviceObject);
		G::DeviceObject = 0;
	}

	SEH_END();

	return STATUS_SUCCESS;
}

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	BOOLEAN  deviceCreated		= FALSE;
	BOOLEAN  symbolLinkCreated	= FALSE;
	NTSTATUS Status				= STATUS_UNSUCCESSFUL;

	SEH_START();

	DriverObject->DriverUnload	= ReCa<PDRIVER_UNLOAD>(DriverUnload);

	// Basic Driver Entry, Creating Device, etc do not have any IOCTL
	//
	RtlInitUnicodeString(&G::DeviceName, L"\\Device\\LockDriverMemory");

	Status = IoCreateDevice(DriverObject, 0, &G::DeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &G::DeviceObject);

	if (!NT_SUCCESS(Status) || !G::DeviceObject) {
		DBE("IoCreateDevice Error (%lx)", Status);
		goto Cleanup;
	} deviceCreated = true;

	Status = IoCreateSymbolicLink(&G::SymbolicLinkName, &G::DeviceName);

	if (!NT_SUCCESS(Status)) {
		DBE("IoCreateSymbolicLink Error (%lx)", Status);
		goto Cleanup;
	} symbolLinkCreated = true;

	DBP("Driver Loaded");

	if (G::DriverEntry)
	{
		G::DriverImageBase = ReCa<UINT_PTR>(G::DriverEntry->DllBase);
		G::DriverImageSize = G::DriverEntry->SizeOfImage;

		G::DriverObject = DriverObject;
		G::DeviceObject = DriverObject->DeviceObject;
	}


	Status = Helper::LockDriverMemory();
	
	if (!NT_SUCCESS(Status))
	{
		DBE("Lock Error (%lx)", Status);
		goto Cleanup;
	}

	return STATUS_SUCCESS;

Cleanup:
	if (symbolLinkCreated)
	{
		IoDeleteSymbolicLink(&G::SymbolicLinkName);
	}

	if (deviceCreated && G::DeviceObject)
	{
		IoDeleteDevice(G::DeviceObject);
	}

	SEH_END();

	return Status;
}


