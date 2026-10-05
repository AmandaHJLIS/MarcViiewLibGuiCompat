#include <gccore.h>
#include <wiiuse/wpad.h>
#include "drivers/ogc/OgcVideoDriver.h"

int main(int, char **)
{
    WPAD_Init();

    OgcVideoDriver video;
    video.init(640, 480);

    while (SYS_MainLoop())
    {
        WPAD_ScanPads();

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;

        video.clearScreen({40, 40, 40, 255});
        video.getImageRenderer()->drawRectangle(
            160.0f, 120.0f, 320.0f, 240.0f,
            {40, 160, 220, 255});

        video.render();
    }

    return 0;
}
