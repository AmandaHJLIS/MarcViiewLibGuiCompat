#include <gccore.h>

#include "drivers/ogc/wii/WiiPlatform.h"
#include "libgui/Gui.h"

WiiPlatform platformInstance;
Platform* platform = &platformInstance;

static void showColour(const PixelColor& color, int frames)
{
    for (int i = 0; i < frames && SYS_MainLoop(); ++i)
    {
        platform->getVideo()->clearScreen(color);
        platform->getVideo()->render();
    }
}

int main(int, char **)
{
    PlatformConfig config;
    config.canvasWidth = 640;
    config.canvasHeight = 480;

    platform->init(config);

    // Platform startup has already been proven.
    showColour({255, 0, 0, 255}, 120);

    GuiText text("libgui text test", 32, {255, 255, 255, 255});

    // GuiText construction returned successfully.
    showColour({0, 255, 0, 255}, 120);

    // Deliberately do not call setSize(), setPosition(), or draw().
    while (SYS_MainLoop())
    {
        platform->getVideo()->clearScreen({0, 0, 0, 255});
        platform->getVideo()->render();
    }

    return 0;
}
