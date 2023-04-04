#pragma once

class device {
	HANDLE	m_handle = INVALID_HANDLE_VALUE;

public:
	device() {}
	device( const wchar_t * name ) { open( name ); }
	virtual ~device() { close(); }

	explicit operator bool () const { return m_handle != INVALID_HANDLE_VALUE; }

	HANDLE get_handle() { return m_handle; }

	bool open( const wchar_t * name ) {
		if ( m_handle != INVALID_HANDLE_VALUE ) close();

		m_handle = CreateFileW( name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
		return m_handle != INVALID_HANDLE_VALUE;
	}

	void close() {
		if ( m_handle != INVALID_HANDLE_VALUE ) {
			CloseHandle( m_handle );
			m_handle = INVALID_HANDLE_VALUE;
		}
	}
}; // class device
