/****************************************************************************
 * MarcViiewLibGuiTest - legacy libogc compatibility
 * Minimal OgcSmbDriver stub for the standalone libgui integration test.
 *
 * The test toolchain does not provide libsmb2, so SMB functionality is
 * intentionally unavailable. This file supplies the class/vtable required
 * by OgcFileSystemDriver without pulling in the optional network-share
 * implementation.
 ***************************************************************************/
#include <cstring>

#include "drivers/ogc/OgcSmbDriver.h"

smb2_context * OgcSmbDriver::ctx = nullptr;

void OgcSmbDriver::init()
{
}

void OgcSmbDriver::shutdown()
{
    disconnect();
}

bool OgcSmbDriver::isNetworkUp() const
{
    return false;
}

bool OgcSmbDriver::ensureNetworkUp()
{
    return false;
}

SmbConnectResult OgcSmbDriver::connect(const SmbShareInfo &)
{
    return SmbConnectResult::NetworkUnavailable;
}

void OgcSmbDriver::disconnect()
{
    ctx = nullptr;
    std::memset(&current, 0, sizeof(current));
    devoptabAdded = false;
}

const char * OgcSmbDriver::connectResultMessage(SmbConnectResult result) const
{
    switch(result)
    {
        case SmbConnectResult::Success:            return "Connected.";
        case SmbConnectResult::InvalidSettings:    return "Network share host/name is blank.";
        case SmbConnectResult::NetworkUnavailable: return "SMB unavailable in legacy test.";
        case SmbConnectResult::ConnectFailed:      return "SMB unavailable in legacy test.";
        default:                                   return "Unknown network share error.";
    }
}

const char * OgcSmbDriver::getLastError() const
{
    return "SMB support is not available in this legacy libogc test build.";
}
