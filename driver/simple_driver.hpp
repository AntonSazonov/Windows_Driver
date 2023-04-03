#pragma once

#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) \
  (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif

#ifndef FILE_DEVICE_UNKNOWN
#define FILE_DEVICE_UNKNOWN		0x00000022
#endif

#define MY_DEVICE_TYPE			FILE_DEVICE_UNKNOWN

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.
#define MY_DEVICE_IOCTL_MAP \
	CTL_CODE( MY_DEVICE_TYPE, 0x800u, 0/*METHOD_BUFFERED*/, 0 /*FILE_ANY_ACCESS*/ )

#define MY_DEVICE_IOCTL_UNMAP \
	CTL_CODE( MY_DEVICE_TYPE, 0x801u, 0/*METHOD_BUFFERED*/, 0 /*FILE_ANY_ACCESS*/ )


struct PHYS_MEM_DESC {
	PVOID	p_addr;
	ULONG	u_size;
}; // struct PHYS_MEM_DESC
