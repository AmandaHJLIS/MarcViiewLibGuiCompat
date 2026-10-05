/****************************************************************************
 * MarcViiewLibGuiCompat
 * Minimal hardware startup diagnostic.
 *
 * This first-stage test intentionally avoids:
 *   - FreeType/font loading
 *   - language data
 *   - audio
 *   - Wii Remote/input initialisation
 *   - image decode scratch allocation
 *   - bundled libgui assets
 *
 * The purpose is to determine whether a real Wii can initialise the
 * platform/video path and render a tiny GUI without the larger startup
 * workload used by the full libgui test.
 ***************************************************************************/

#include "drivers/Platform.h"
#include "drivers/ogc/wii/WiiPlatform.h"
#include "libgui/Gui.h"

static WiiPlatform platformInstance;
Platform* platform = &platformInstance;

int main(int, char **)
{
    PlatformConfig config;
    config.canvasWidth = 640;
    config.canvasHeight = 480;

    platform->init(config);

    GuiWindow mainWindow(
        platform->getVideo()->getScreenWidth(),
        platform->getVideo()->getScreenHeight());

    /*
     * Use only solid GUI primitives here. No font renderer, external
     * resources, decoder scratch buffer, audio, or input threads.
     *
     * If this reaches the screen on real hardware, platform/video/GUI
     * startup is fundamentally working and we can add the heavier
     * subsystems back one at a time.
     */
    GuiImage background(
        platform->getVideo()->getScreenWidth(),
        platform->getVideo()->getScreenHeight(),
        (PixelColor){40, 40, 40, 255});

    GuiImage marker(
        240,
        100,
        (PixelColor){180, 180, 180, 255});

    marker.setAlignment(ALIGN_H::CENTRE, ALIGN_V::CENTRE);
    marker.setPosition(0, 0);

    mainWindow.append(&background);
    mainWindow.append(&marker);

    while (!platform->shouldExit())
    {
        mainWindow.draw();
        platform->getVideo()->render();
    }

    platform->requestExit();
}
