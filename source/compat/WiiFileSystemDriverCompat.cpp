/****************************************************************************
 * libgui legacy libogc compatibility
 * WiiFileSystemDriver.cpp
 *
 * Compatibility copy of upstream libgui's Wii filesystem driver for the
 * older libogc toolchain used by this standalone integration test.
 *
 * Legacy libogc exposes the built-in Wii disc interfaces as const
 * DISC_INTERFACE objects, while libgui's current OgcFileSystemDriver API
 * stores mutable DISC_INTERFACE pointers. The driver never mutates these
 * interface descriptors, so const_cast bridges that API difference.
 ***************************************************************************/
#include <sdcard/wiisd_io.h>
#include <ogc/usb.h>
#include <ogc/usbstorage.h>
#include <di/di.h>

#include "WiiFileSystemDriver.h"
#include "WiiUsbMulti.h"

static DISC_INTERFACE * GetDiscSd()   { return const_cast<DISC_INTERFACE *>(&__io_wiisd); }
static DISC_INTERFACE * GetDiscUsb1() { return WiiUsbMulti::getInterface(0); }
static DISC_INTERFACE * GetDiscUsb2() { return WiiUsbMulti::getInterface(1); }
static DISC_INTERFACE * GetDiscUsb3() { return WiiUsbMulti::getInterface(2); }

static const OgcFatSlotDescriptor wiiFatSlots[] =
{
	{ DEVICE_SD,   GetDiscSd,   "sd",   "sd:/",   "SD Card",             "SD card not found!",     true, true, true },
	{ DEVICE_USB,  GetDiscUsb1, "usb",  "usb:/",  "USB Mass Storage",    "USB drive not found!",   true, true, true },
	{ DEVICE_USB2, GetDiscUsb2, "usb2", "usb2:/", "USB Mass Storage 2",  "USB drive 2 not found!", true, true, true },
	{ DEVICE_USB3, GetDiscUsb3, "usb3", "usb3:/", "USB Mass Storage 3",  "USB drive 3 not found!", true, true, true },
};

void WiiFileSystemDriver::init()
{
	DI_Init();
	USB_Initialize();
	USBStorage_Initialize();
	WiiUsbMulti::init();
	WiiUsbMulti::scan();

	fatSlots     = wiiFatSlots;
	fatSlotCount = sizeof(wiiFatSlots) / sizeof(wiiFatSlots[0]);
	dvdDisc      = const_cast<DISC_INTERFACE *>(&__io_wiidvd);
	unsupportedFormatMessage = "Unsupported format - FAT32 is recommended.";

	smbDriver.init();

	initFatSlotsAndAutoMount();
}

void WiiFileSystemDriver::shutdownPlatform()
{
	WiiUsbMulti::shutdown();
	USBStorage_Deinitialize();
	DI_Close();
}

bool WiiFileSystemDriver::pollPlatformExtra()
{
	return WiiUsbMulti::scan();
}

void WiiFileSystemDriver::prepareMount(int deviceId)
{
	if(deviceId == DEVICE_USB || deviceId == DEVICE_USB2 || deviceId == DEVICE_USB3)
		WiiUsbMulti::scan();
}

const int * WiiFileSystemDriver::getValidLoadDevices(int & outCount) const
{
	static const int devices[] = { DEVICE_AUTO, DEVICE_SD, DEVICE_USB, DEVICE_USB2, DEVICE_USB3, DEVICE_DVD, DEVICE_SMB };
	outCount = sizeof(devices) / sizeof(devices[0]);
	return devices;
}

const int * WiiFileSystemDriver::getValidSaveDevices(int & outCount) const
{
	static const int devices[] = { DEVICE_AUTO, DEVICE_SD, DEVICE_USB, DEVICE_USB2, DEVICE_USB3, DEVICE_SMB };
	outCount = sizeof(devices) / sizeof(devices[0]);
	return devices;
}
