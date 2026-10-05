#include <gccore.h>
#include <wiiuse/wpad.h>
#include <stdint.h>
#include <stdlib.h>

#include "drivers/ogc/OgcVideoDriver.h"

int main(int, char **)
{
    WPAD_Init();

    OgcVideoDriver video;
    video.init(640, 480);

    ImageRenderer* image = video.getImageRenderer();

    const int textureWidth = 32;
    const int textureHeight = 32;
    uint8_t rgba[textureWidth * textureHeight * 4];

    // A deliberately simple test pattern: red background with a white cross.
    for (int y = 0; y < textureHeight; ++y) {
        for (int x = 0; x < textureWidth; ++x) {
            const bool cross = (x >= 14 && x <= 17) || (y >= 14 && y <= 17);
            uint8_t* pixel = &rgba[(y * textureWidth + x) * 4];

            pixel[0] = cross ? 255 : 220;
            pixel[1] = cross ? 255 : 40;
            pixel[2] = cross ? 255 : 40;
            pixel[3] = 255;
        }
    }

    void* texture = image->createTexture(textureWidth, textureHeight);
    image->loadTextureData(texture, rgba, textureWidth, textureHeight);

    while (SYS_MainLoop())
    {
        WPAD_ScanPads();

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;

        video.clearScreen({40, 40, 40, 255});

        // Keep the known-good rectangle as a reference.
        image->drawRectangle(
            80.0f, 120.0f, 220.0f, 240.0f,
            {40, 160, 220, 255});

        // Draw the newly tested texture beside it.
        image->drawTexture(
            texture,
            400.0f, 240.0f,
            textureWidth, textureHeight,
            0.0f, 8.0f, 8.0f, 255);

        video.render();
    }

    image->destroyTexture(texture);

    return 0;
}
