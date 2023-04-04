#pragma once

// Service Control Manager
class SCM {
	SC_HANDLE	g_manager	= nullptr;

public:
	SCM() {
		g_manager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	}

	virtual ~SCM() {
		if ( g_manager ) CloseServiceHandle( g_manager );
	}

	explicit operator bool () const { return g_manager != nullptr; }

	bool load_and_start_driver( const wchar_t * name, const wchar_t * full_file_path ) {
		if ( g_manager ) {
			SC_HANDLE service = CreateServiceW( g_manager, name, name, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, full_file_path, NULL, NULL, NULL, NULL, NULL );
			if ( !service && GetLastError() == ERROR_SERVICE_EXISTS ) {
				service = OpenServiceW( g_manager, name, SERVICE_ALL_ACCESS );
			}
			if ( !service ) return false;

			if ( !StartService( service, 0, NULL ) ) {
				if ( GetLastError() != ERROR_SERVICE_ALREADY_RUNNING ) return false;
				//printf( "Already loaded: %s\n", cpDriverPath );
			}
		}
		return true;
	}

	bool stop_and_unload_driver( const wchar_t * name ) {
		bool result = false;
		if ( SC_HANDLE service = OpenServiceW( g_manager, name, SERVICE_ALL_ACCESS ) ) {
			SERVICE_STATUS status;
			if ( ControlService( service, SERVICE_CONTROL_STOP, &status ) ) {
				result = DeleteService( service );
			}
			CloseServiceHandle( service );
		}
		return result;
	}
}; // class SCM
