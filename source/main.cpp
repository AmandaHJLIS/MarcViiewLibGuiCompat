#include <gccore.h>

#include "drivers/ogc/OgcVideoDriver.h"

static OgcVideoDriver video;

static void showColour(const PixelColor& color, int frames)
{
    for (int i = 0; i < frames && SYS_MainLoop(); ++i)
    {
        video.clearScreen(color);
        video.render();
    }
}

int main(int, char **)
{
    // Bypass WiiPlatform entirely. This isolates the compatibility video
    // driver from thread, input, filesystem and all other startup code.
    video.init(640, 480);

    // RED = OgcVideoDriver::init() returned and the GX path can present.
    showColour({255, 0, 0, 255}, 180);

    // GREEN = continued direct video rendering is alive.
    showColour({0, 255, 0, 255}, 180);

    // BLUE = stable direct video loop.
    while (SYS_MainLoop())
    {
        video.clearScreen({0, 0, 255, 255});
        video.render();
    }

    video.shutdown();
    return 0;
}
