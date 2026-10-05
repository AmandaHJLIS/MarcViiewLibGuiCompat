#include <gccore.h>

#include "drivers/ogc/wii/WiiPlatform.h"
#include "drivers/InputController.h"

WiiPlatform platformInstance;
Platform* platform = &platformInstance;

static void showCheckpoint(const PixelColor& color, int frames)
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

    // Checkpoint 1: platform initialization returned successfully.
    platform->init(config);

    // RED = platform init returned successfully.
    showCheckpoint({255, 0, 0, 255}, 120);

    // Checkpoint 2: the input driver exists and its first update returned.
    platform->getInput()->update();

    // GREEN = first input update returned successfully.
    showCheckpoint({0, 255, 0, 255}, 120);

    // Checkpoint 3: minimal input loop, with no libgui text/image widgets.
    // BLUE means repeated input updates and video rendering are alive.
    while (SYS_MainLoop())
    {
        platform->getInput()->update();

        InputController* pad = controller[0];

        if (pad->isPressed(INPUT_BTN_HOME))
            break;

        platform->getVideo()->clearScreen({0, 0, 255, 255});
        platform->getVideo()->render();
    }

    platform->requestExit();
    return 0;
}
