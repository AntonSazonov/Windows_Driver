#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//#include <winnt.h>
//#include <winternl.h>

#include <cstdio>
#include "SCM.hpp"
#include "device.hpp"

static void print_last_error( const char * p_title, int err = 0 ) {
	char * p_msg;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err ? err : GetLastError(), MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), (LPTSTR)&p_msg, 0, nullptr );
	fprintf( stderr, "%s: %s", p_title, p_msg );
	LocalFree( p_msg );
}

int main() {
	SCM		scm;
	device	dev;

	if ( dev.open( "\\\\.\\Simple_Driver_Example" ) ) {
		printf( "Opened.\n" );
	} else {
		printf( "Not registered/started. Trying to load driver...\n" );
	}

	// If not registered/started...
	if ( !dev ) {

		// Get driver full path.
		char full_driver_path[MAX_PATH] = {};
		GetCurrentDirectory( MAX_PATH, full_driver_path );
		strcat( full_driver_path, "\\simple_driver.sys" );

		printf( "Loading: %s\n", full_driver_path );
		if ( !scm.load_and_start_driver( "simple_driver", full_driver_path ) ) {
			print_last_error( "Load error" );
			return 1;
		}

		// Try to open it again...
		if ( !dev.open( "\\\\.\\Simple_Driver_Example" ) ) {
			print_last_error( "Open" );
			return 2;
		}

		printf( "Opened. Check debug message log.\n" );

	}


	char	data[8]			= { 1, 2, 3, 4, 5, 5, 5, 0 };
	DWORD	dwBytesReturned	= 0;
	BOOL r = DeviceIoControl( dev.get_handle(),
		0x1234,
		data, sizeof( data ),	// in
		nullptr, 0,				// out
		&dwBytesReturned, nullptr );

	printf( "DeviceIoControl(): %d, ret. bytes: %d\n", int(r), int(dwBytesReturned) );

	// Just wait a little...
	Sleep( 3000 );

	dev.close();

	printf( "Unloading...\n" );
	if ( !scm.stop_and_unload_driver( "simple_driver" ) ) {
		print_last_error( "Unloading error" );
	}

	system( "pause" );
	return 0;
}
