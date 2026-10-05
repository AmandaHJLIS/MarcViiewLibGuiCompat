/****************************************************************************
 * MarcViiewLibGuiCompat - isolated OgcVideoDriver diagnostic
 *
 * This test adds exactly one compatibility layer on top of the previously
 * proven raw VIDEO/GX path: upstream libgui's OgcVideoDriver interface,
 * implemented by OgcVideoDriverCompat.cpp for legacy libogc.
 ***************************************************************************/

#include <stdlib.h>\n#include <gccore.h>
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

        // Draw a deliberately simple quad using the GX state initialized by
        // OgcVideoDriver. The driver itself does not draw scene geometry;
        // libgui normally does that through its renderers.
        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(0.0f, 0.0f, 0.0f);
        GX_Color4u8(40, 40, 40, 255);
        GX_Position3f32(640.0f, 0.0f, 0.0f);
        GX_Color4u8(40, 40, 40, 255);
        GX_Position3f32(640.0f, 480.0f, 0.0f);
        GX_Color4u8(40, 40, 40, 255);
        GX_Position3f32(0.0f, 480.0f, 0.0f);
        GX_Color4u8(40, 40, 40, 255);
        GX_End();

        video.render();
    }

    video.shutdown();
    return 0;
}
