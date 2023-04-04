#pragma once

#define DRIVER_FILE_NAME_STRING	L"san_ram_drv.sys"
#define SERVICE_NAME_STRING		L"SAN_RAM_Drv"
#define NTDEVICE_NAME_STRING	L"\\Device\\" SERVICE_NAME_STRING
#define SYMBOLIC_NAME_STRING	L"\\DosDevices\\" SERVICE_NAME_STRING
#define DEVICE_NAME_STRING		L"\\\\.\\" SERVICE_NAME_STRING

#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) \
  (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif

#ifndef FILE_DEVICE_UNKNOWN
#define FILE_DEVICE_UNKNOWN		0x00000022
#endif

#define SAN_RAM_DEVICE_TYPE		FILE_DEVICE_UNKNOWN

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.
#define SAN_RAM_IOCTL_MAP \
	CTL_CODE( SAN_RAM_DEVICE_TYPE, 0x800u, 0/*METHOD_BUFFERED*/, 0 /*FILE_ANY_ACCESS*/ )

#define SAN_RAM_IOCTL_UNMAP \
	CTL_CODE( SAN_RAM_DEVICE_TYPE, 0x801u, 0/*METHOD_BUFFERED*/, 0 /*FILE_ANY_ACCESS*/ )


struct PHYS_MEM_DESC {
	PVOID	p_addr;
	ULONG	u_size;
}; // struct PHYS_MEM_DESC
