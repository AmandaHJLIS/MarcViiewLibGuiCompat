#include <gccore.h>
#include <wiiuse/wpad.h>

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

static bool homePressed()
{
    WPAD_ScanPads();

    for (int i = 0; i < WPAD_MAX_WIIMOTES; ++i)
    {
        if (WPAD_ButtonsDown(i) & WPAD_BUTTON_HOME)
            return true;
    }

    return false;
}

int main(int, char **)
{
    PlatformConfig config;
    config.canvasWidth = 640;
    config.canvasHeight = 480;

    platform->init(config);

    showColour({255, 0, 0, 255}, 120);

    GuiText text("libgui text test", 32, {255, 255, 255, 255});

    showColour({0, 255, 0, 255}, 120);

    text.setSize(640, 80);

    showColour({0, 0, 255, 255}, 120);

    // Test only setPosition().
    text.setPosition(0, 200);

    // setPosition() returned successfully.
    showColour({255, 255, 0, 255}, 120);

    while (SYS_MainLoop())
    {
        if (homePressed())
            break;

        platform->getVideo()->clearScreen({0, 0, 0, 255});
        platform->getVideo()->render();
    }

    return 0;
}
