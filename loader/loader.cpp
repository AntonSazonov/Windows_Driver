#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <cctype>

#include "SCM.hpp"
#include "device.hpp"

#include "../driver/san_ram_drv.hpp"

namespace {

void print_last_error( const char * p_title, int err = 0 ) {
	char * p_msg;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err ? err : GetLastError(), MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), (LPTSTR)&p_msg, 0, nullptr );
	fprintf( stderr, "%s: %s", p_title, p_msg );
	LocalFree( p_msg );
}

} // namespace

int main() {
	SCM		scm; // Service Control Manager
	device	dev; // Device

	// Check if device driver already loaded and started...
	if ( dev.open( DEVICE_NAME_STRING ) ) {
		printf( "Opened.\n" );
	} else {
		printf( "Not registered/started. Trying to load driver...\n" );
	}

	// If not registered/loaded/started...
	if ( !dev ) {

		// Get driver full path
		wchar_t full_driver_path[MAX_PATH] = {};
		GetFullPathNameW( DRIVER_FILE_NAME_STRING, MAX_PATH, full_driver_path, NULL );

		printf( "Loading: %ls\n", DRIVER_FILE_NAME_STRING );
		if ( !scm.load_and_start_driver( SERVICE_NAME_STRING, full_driver_path ) ) {
			print_last_error( "Load error" );
			return 1;
		}

		// Try to open it again...
		if ( !dev.open( DEVICE_NAME_STRING ) ) {
			print_last_error( "dev.open()" );
			scm.stop_and_unload_driver( SERVICE_NAME_STRING );
			return 2;
		}

		printf( "Opened. Check debug message log.\n" );
	}

	// Setup physical memory region...
	PHYS_MEM_DESC mem;
	mem.p_addr	= (PVOID)0xc0000;
	mem.u_size	= 640;

	// Map it...
	PVOID p_mapped = nullptr;
	DWORD dwRet = 0;
	DeviceIoControl( dev.get_handle(),
		SAN_RAM_IOCTL_MAP,
		&mem, sizeof( PHYS_MEM_DESC ),	// in
		&p_mapped, sizeof( PVOID ),		// out
		&dwRet, nullptr );

	if ( p_mapped ) {
		printf( "First %lu printable characters of Video BIOS:\n\n", mem.u_size );
		const char * p = (const char *)p_mapped;

		printf( "  " );
		for ( unsigned i = 0; i < mem.u_size; i++ ) {
			printf( "%c", isprint( *p ) ? *p : '.' );
			if ( !((i + 1) % 80) ) printf( "\n  " );
			p++;
		}
		printf( "\n" );

		// Unmap memory
		DeviceIoControl( dev.get_handle(), SAN_RAM_IOCTL_UNMAP, NULL, 0, NULL, 0, &dwRet, nullptr );
	}

	// Close device...
	dev.close();

	// Stop and unregister...
	printf( "Unloading...\n" );
	if ( !scm.stop_and_unload_driver( SERVICE_NAME_STRING ) ) {
		print_last_error( "Unloading error" );
	}

	// We run as Admin in detached console, so pause is needed.
	system( "pause" );
	return 0;
}
