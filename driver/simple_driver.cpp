#define DBG 1
#define IRPMJFUNCDESC
#include <ntddk.h>
#include <ksdebug.h>

extern "C" DRIVER_INITIALIZE	DriverEntry;
extern "C" DRIVER_UNLOAD		DriverUnload;

#define NTDEVICE_NAME_STRING      L"\\Device\\Simple_Driver_Example"
#define SYMBOLIC_NAME_STRING      L"\\DosDevices\\Simple_Driver_Example"


extern "C" NTSTATUS EventCreateClose( PDEVICE_OBJECT /*p_DeviceObject*/, PIRP p_Irp ) {
	PIO_STACK_LOCATION p_io = IoGetCurrentIrpStackLocation( p_Irp );

	// Check out-of-bounds...
#if 1
	if ( p_io->MajorFunction <= sizeof( IrpMjFuncDesc ) / sizeof( IrpMjFuncDesc[0] ) ) {
		DbgPrint( "%s\n", IrpMjFuncDesc[p_io->MajorFunction] );
	} else {
		DbgPrint( "Unknown Major function: %08x\n", p_io->MajorFunction );
	}
#endif

	if ( p_io->MajorFunction == IRP_MJ_DEVICE_CONTROL ) {
		DbgPrint( "IO code: %08x\n", p_io->Parameters.DeviceIoControl.IoControlCode );
		DbgPrint( "                 ptr = %p\n", p_Irp->AssociatedIrp.SystemBuffer );
		DbgPrint( "  OutputBufferLength = %d\n", p_io->Parameters.DeviceIoControl.OutputBufferLength );
		DbgPrint( "   InputBufferLength = %d\n", p_io->Parameters.DeviceIoControl.InputBufferLength );

		char * p = (char *)p_Irp->AssociatedIrp.SystemBuffer;
		DbgPrint( " " );
		for ( int i = 0; i < p_io->Parameters.DeviceIoControl.InputBufferLength; i++ ) {
			DbgPrint( " %d", int(p[i]) );
		}
		DbgPrint( "\n" );
	}

#if 0
	switch ( p_io->MajorFunction ) {
		case IRP_MJ_CREATE:
			DbgPrint( "IRP_MJ_CREATE\n" );
			break;

		case IRP_MJ_CLOSE:
			DbgPrint( "IRP_MJ_CLOSE\n" );
			break;

		default:
			DbgPrint( "Unknown IRP (%08x)\n", p_io->MajorFunction );
			DbgPrint( "%s\n", IrpMjFuncDesc[p_io->MajorFunction] );
			break;
	}
#endif

	p_Irp->IoStatus.Status		= STATUS_SUCCESS;
	p_Irp->IoStatus.Information	= 0;
	IoCompleteRequest( p_Irp, IO_NO_INCREMENT );
	return STATUS_SUCCESS;
}

extern "C" NTSTATUS EventCleanup( PDEVICE_OBJECT /*DeviceObjec*/, PIRP /*Irp*/ ) {
	DbgPrint( "%s\n", __PRETTY_FUNCTION__ );
	return STATUS_SUCCESS;
}

extern "C" NTSTATUS EventDispatchIoControl( PDEVICE_OBJECT /*DeviceObject*/, PIRP /*Irp*/ ) {
	DbgPrint( "%s\n", __PRETTY_FUNCTION__ );
	return STATUS_SUCCESS;
}

extern "C" NTSTATUS DriverEntry( PDRIVER_OBJECT p_DriverObject, PUNICODE_STRING /*RegistryPath*/ ) {

	p_DriverObject->MajorFunction[IRP_MJ_CREATE]			= EventCreateClose;
	p_DriverObject->MajorFunction[IRP_MJ_CLOSE]				= EventCreateClose;
	p_DriverObject->MajorFunction[IRP_MJ_CLEANUP]			= EventCreateClose;//EventCleanup;
	p_DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]	= EventCreateClose;//EventDispatchIoControl;
	p_DriverObject->DriverUnload							= DriverUnload;

	UNICODE_STRING ntDeviceName;
	RtlInitUnicodeString( &ntDeviceName, NTDEVICE_NAME_STRING );

    PDEVICE_OBJECT p_DeviceObject;
	NTSTATUS status = IoCreateDevice(
		p_DriverObject,
		0,//sizeof( DEVICE_EXTENSION ),
		&ntDeviceName,
		FILE_DEVICE_UNKNOWN,
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

	DbgPrint( "%s: p_DeviceObject = %p\n", __PRETTY_FUNCTION__, p_DeviceObject );
	return STATUS_SUCCESS;
}

extern "C" void DriverUnload( PDRIVER_OBJECT p_DriverObject ) {
	PDEVICE_OBJECT p_DeviceObject = p_DriverObject->DeviceObject;
	DbgPrint( "%s: p_DeviceObject = %p\n", __PRETTY_FUNCTION__, p_DeviceObject );

	// Delete the user-mode symbolic link and DeviceObject.
	UNICODE_STRING symbolicLinkName;
	RtlInitUnicodeString( &symbolicLinkName, SYMBOLIC_NAME_STRING );
	IoDeleteSymbolicLink( &symbolicLinkName );
	IoDeleteDevice( p_DeviceObject );
}
