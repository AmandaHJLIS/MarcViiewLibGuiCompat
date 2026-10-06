#include <gccore.h>

#include "drivers/ogc/wii/WiiPlatform.h"
#include "libgui/Gui.h"

WiiPlatform platformInstance;
Platform* platform = &platformInstance;

int main(int, char **)
{
    PlatformConfig config;
    config.canvasWidth = 640;
    config.canvasHeight = 480;

    platform->init(config);

    GuiText text("libgui text test", 32, {255, 255, 255, 255});
    text.setSize(640, 80);
    text.setPosition(0, 200);

    while (SYS_MainLoop())
    {
        platform->getVideo()->clearScreen({0, 0, 0, 255});

        text.draw();

        platform->getVideo()->render();
    }

    return 0;
}
