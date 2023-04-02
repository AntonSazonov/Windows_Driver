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
		Sleep( 1000 );
		dev.close();
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

		printf( "Ok. Check debug message log.\n" );
	}


	Sleep( 3000 );


	printf( "Unloading...\n" );
	if ( !scm.stop_and_unload_driver( "simple_driver" ) ) {
		print_last_error( "Unloading error" );
	}

	system( "pause" );
	return 0;
}
