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

    // Platform startup has already been proven. Show a visible checkpoint
    // before touching GuiText.
    showColour({255, 0, 0, 255}, 120);

    GuiText text("libgui text test", 32, {255, 255, 255, 255});

    // Construction returned successfully.
    showColour({0, 255, 0, 255}, 120);

    text.setSize(640, 80);

    // setSize returned successfully.
    showColour({0, 0, 255, 255}, 120);

    text.setPosition(0, 200);

    // setPosition returned successfully. Do not draw yet.
    showColour({255, 255, 0, 255}, 120);

    while (SYS_MainLoop())
    {
        platform->getVideo()->clearScreen({0, 0, 0, 255});
        platform->getVideo()->render();
    }

    return 0;
}
