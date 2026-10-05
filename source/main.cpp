/****************************************************************************
 * MarcViiewLibGuiCompat - isolated OgcVideoDriver diagnostic
 *
 * This test adds exactly one compatibility layer on top of the previously
 * proven raw VIDEO/GX path: upstream libgui's OgcVideoDriver interface,
 * implemented by OgcVideoDriverCompat.cpp for legacy libogc.
 ***************************************************************************/

#include <stdlib.h>
#include <wiiuse/wpad.h>

#include "drivers/ogc/OgcVideoDriver.h"

int main(int, char **)
{
    WPAD_Init();

    OgcVideoDriver video;
    video.init(640, 480);

    // Keep the scene deliberately simple. If this renders correctly on both
    // Dolphin and real Wii hardware, the compatibility driver's video path
    // is working independently of libgui's other drivers.
    video.clearScreen({40, 40, 40, 255});

    while (SYS_MainLoop())
    {
        WPAD_ScanPads();

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;

        video.render();
    }

    video.shutdown();
    return 0;
}
