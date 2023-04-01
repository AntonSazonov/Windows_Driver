#include <ntddk.h>

extern "C" DRIVER_INITIALIZE	DriverEntry;
extern "C" DRIVER_UNLOAD		DriverUnload;

#if 0
typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
#endif

//__attribute__ ((visibility( "default" )))
extern "C" NTSTATUS DriverEntry( struct _DRIVER_OBJECT * DriverObject, PUNICODE_STRING /*RegistryPath*/ ) {
	DriverObject->DriverUnload = DriverUnload;

	DbgPrint( "%s\n", __PRETTY_FUNCTION__ );

	//std::size_t wcstombs( char* dst, const wchar_t* src, std::size_t len);
	//OutputDebugStringW( "%.*s\n", RegistryPath.Length, RegistryPath.Buffer );

	return STATUS_SUCCESS;
}

//__attribute__ ((visibility( "default" )))
extern "C" VOID DriverUnload( struct _DRIVER_OBJECT * /*DriverObject*/ ) {
	DbgPrint( "%s\n", __PRETTY_FUNCTION__ );
}
