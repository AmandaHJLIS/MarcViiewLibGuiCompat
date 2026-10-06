#include <gccore.h>

#include "drivers/ogc/wii/WiiPlatform.h"

WiiPlatform platformInstance;
Platform* platform = &platformInstance;

int main(int, char **)
{
    PlatformConfig config;
    config.canvasWidth = 640;
    config.canvasHeight = 480;

    platform->init(config);

    // Reaching this loop means WiiPlatform startup returned after
    // the video/input checkpoints in WiiPlatformCompat.cpp.
    while (SYS_MainLoop())
    {
        platform->getVideo()->clearScreen({0, 0, 255, 255});
        platform->getVideo()->render();
    }

    return 0;
}
