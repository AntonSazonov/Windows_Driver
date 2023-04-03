
#include <ntddk.h>
#include "simple_driver.hpp"

extern "C" DRIVER_INITIALIZE	DriverEntry;
extern "C" DRIVER_UNLOAD		DriverUnload;

#define NTDEVICE_NAME_STRING      L"\\Device\\Simple_Driver_Example"
#define SYMBOLIC_NAME_STRING      L"\\DosDevices\\Simple_Driver_Example"

namespace {

ULONG	g_size			= 0;
PVOID	g_map			= nullptr;
PMDL	g_mdl			= nullptr;
PVOID	g_map_locked	= nullptr;

} // namespace

void phys_free() {
	DbgPrint( "%s: unmapping phys.: %p, virt.: %p, size: %lu\n", __PRETTY_FUNCTION__, g_map, g_map_locked, g_size );

	if ( g_map_locked && g_mdl ) {
		MmUnmapLockedPages( g_map_locked, g_mdl );
		g_map_locked	= nullptr;
		g_mdl			= nullptr;
	}

	if ( g_mdl ) {
		IoFreeMdl( g_mdl );
		g_mdl = nullptr;
	}

	if ( g_map && g_size ) {
		MmUnmapIoSpace( g_map, g_size );
		g_map	= nullptr;
		g_size	= 0;
	}
}

NTSTATUS phys_map( PVOID p_addr, ULONG size, PVOID * pp_mapped ) {

	// Free previously mapped memory.
	phys_free();

	if ( !size || !pp_mapped ) return STATUS_INVALID_PARAMETER;

	PHYSICAL_ADDRESS phy_addr;
	phy_addr.QuadPart = (ULONGLONG)p_addr;

	// If space for mapping the range is insufficient, it returns NULL.
	PVOID p_map = MmMapIoSpace( phy_addr, size, MmNonCached );
	if ( !p_map ) {
		DbgPrint( "%s: MmMapIoSpace() error.\n", __PRETTY_FUNCTION__ );
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	g_size = size;

	// If the MDL cannot be allocated, it returns NULL.
	PMDL p_mdl = IoAllocateMdl( p_map, size, FALSE, FALSE, NULL );
	if ( !p_mdl ) {
		DbgPrint( "%s: IoAllocateMdl() error.\n", __PRETTY_FUNCTION__ );
		phys_free();
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// Describe the underlying physical pages...
	MmBuildMdlForNonPagedPool( p_mdl );

	PVOID p_map_locked = MmMapLockedPagesSpecifyCache(
		p_mdl,					// Memory Descriptor List.
		UserMode,				// Access mode: KernelMode or UserMode.
		MmNonCached,			// Cache Type.
		NULL,					// Requested Address. Set to NULL to allow the system to choose the starting address.
		FALSE, 					// Drivers must set this parameter to FALSE.
		NormalPagePriority );	// Priority.

	if ( !p_map_locked ) {
		DbgPrint( "%s: MmMapLockedPagesSpecifyCache() error.\n", __PRETTY_FUNCTION__ );
		phys_free();
		return STATUS_INSUFFICIENT_RESOURCES;
	}


	*pp_mapped = p_map_locked;

	g_map = p_map;
	g_mdl = p_mdl;
	g_map_locked = p_map_locked;

	return STATUS_SUCCESS;
}


extern "C" NTSTATUS Create( PDEVICE_OBJECT /*p_DeviceObject*/, PIRP p_Irp ) {
	DbgPrint( "%s\n", __PRETTY_FUNCTION__ );
	p_Irp->IoStatus.Status		= STATUS_SUCCESS;
	p_Irp->IoStatus.Information	= 0;
	IoCompleteRequest( p_Irp, IO_NO_INCREMENT );
	return STATUS_SUCCESS;
}

extern "C" NTSTATUS Close( PDEVICE_OBJECT /*p_DeviceObject*/, PIRP p_Irp ) {
	DbgPrint( "%s\n", __PRETTY_FUNCTION__ );
	p_Irp->IoStatus.Status		= STATUS_SUCCESS;
	p_Irp->IoStatus.Information	= 0;
	IoCompleteRequest( p_Irp, IO_NO_INCREMENT );
	return STATUS_SUCCESS;
}

extern "C" NTSTATUS Cleanup( PDEVICE_OBJECT /*p_DeviceObject*/, PIRP p_Irp ) {
	DbgPrint( "%s\n", __PRETTY_FUNCTION__ );

	phys_free(); // Free if any allocated...

	p_Irp->IoStatus.Status		= STATUS_SUCCESS;
	p_Irp->IoStatus.Information	= 0;
	IoCompleteRequest( p_Irp, IO_NO_INCREMENT );
	return STATUS_SUCCESS;
}

extern "C" NTSTATUS IoControl( PDEVICE_OBJECT /*p_DeviceObject*/, PIRP p_Irp ) {
	DbgPrint( "%s\n", __PRETTY_FUNCTION__ );

	// Default status is successful.
	p_Irp->IoStatus.Status = STATUS_SUCCESS;
	p_Irp->IoStatus.Information	= 0;

	PIO_STACK_LOCATION p_io = IoGetCurrentIrpStackLocation( p_Irp );

	PVOID	p_buffer		= p_Irp->AssociatedIrp.SystemBuffer;
	ULONG	u_buffer_out	= p_io->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG	u_buffer_in		= p_io->Parameters.DeviceIoControl.InputBufferLength;

	ULONG code = p_io->Parameters.DeviceIoControl.IoControlCode;
	switch ( code ) {
		case MY_DEVICE_IOCTL_MAP: {

			// Check for correct parameters...
			if ( u_buffer_in != sizeof( PHYS_MEM_DESC ) || u_buffer_out != sizeof( PVOID ) ) {
				DbgPrint( "%s: Invalid parameters.\n", __PRETTY_FUNCTION__ );
				p_Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
				break;
			}

			PHYS_MEM_DESC * p_phy_mem = (PHYS_MEM_DESC *)p_buffer;

			PVOID p_mapped = nullptr;
			p_Irp->IoStatus.Status = phys_map( p_phy_mem->p_addr, p_phy_mem->u_size, &p_mapped );
			if ( p_Irp->IoStatus.Status != STATUS_SUCCESS ) break;

			RtlCopyMemory( p_buffer, &p_mapped, sizeof( PVOID ) );

			p_Irp->IoStatus.Information = sizeof( PVOID );

			DbgPrint( "%s: mapped at %p.\n", __PRETTY_FUNCTION__, p_mapped );
		} break;

		case MY_DEVICE_IOCTL_UNMAP:
			DbgPrint( "%s: MY_DEVICE_IOCTL_UNMAP\n", __PRETTY_FUNCTION__ );

			// Unmap last mapped.
			phys_free();
			break;

		default:
			DbgPrint( "%s: Unknown IO control code %08x.\n", __PRETTY_FUNCTION__, code );
			break;
	}

	IoCompleteRequest( p_Irp, IO_NO_INCREMENT );
	return p_Irp->IoStatus.Status;
}


extern "C" void Unload( PDRIVER_OBJECT p_DriverObject ) {
	PDEVICE_OBJECT p_DeviceObject = p_DriverObject->DeviceObject;
	DbgPrint( "%s\n", __PRETTY_FUNCTION__ );

	// Delete the user-mode symbolic link and DeviceObject.
	UNICODE_STRING symbolicLinkName;
	RtlInitUnicodeString( &symbolicLinkName, SYMBOLIC_NAME_STRING );
	IoDeleteSymbolicLink( &symbolicLinkName );
	IoDeleteDevice( p_DeviceObject );
}

extern "C" NTSTATUS DriverEntry( PDRIVER_OBJECT p_DriverObject, PUNICODE_STRING /*RegistryPath*/ ) {

	p_DriverObject->MajorFunction[IRP_MJ_CREATE]			= Create;
	p_DriverObject->MajorFunction[IRP_MJ_CLOSE]				= Close;
	p_DriverObject->MajorFunction[IRP_MJ_CLEANUP]			= Cleanup;
	p_DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]	= IoControl;
	p_DriverObject->DriverUnload							= Unload;

	UNICODE_STRING ntDeviceName;
	RtlInitUnicodeString( &ntDeviceName, NTDEVICE_NAME_STRING );

    PDEVICE_OBJECT p_DeviceObject;
	NTSTATUS status = IoCreateDevice(
		p_DriverObject,
		0,//sizeof( DEVICE_EXTENSION ),
		&ntDeviceName,
		MY_DEVICE_TYPE,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&p_DeviceObject );
	if ( !NT_SUCCESS( status ) ) {
		DbgPrint( "IoCreateDevice returned 0x%x\n", status );
		return status;
	}

	// Create a symbolic link for userapp to interact with the driver.
	UNICODE_STRING symbolicLinkName;
	RtlInitUnicodeString( &symbolicLinkName, SYMBOLIC_NAME_STRING );
	status = IoCreateSymbolicLink( &symbolicLinkName, &ntDeviceName );
	if ( !NT_SUCCESS( status ) ) {
		IoDeleteDevice( p_DeviceObject );
		DbgPrint( "IoCreateSymbolicLink returned 0x%x\n", status );

		// Remove user-mode symbolic link if it already present.
		if ( status == 0xc0000035 ) {
			UNICODE_STRING symbolicLinkName;
			RtlInitUnicodeString( &symbolicLinkName, SYMBOLIC_NAME_STRING );
			IoDeleteSymbolicLink( &symbolicLinkName );
		}
		return status;
	}

	DbgPrint( "%s\n", __PRETTY_FUNCTION__ );
	return STATUS_SUCCESS;
}
