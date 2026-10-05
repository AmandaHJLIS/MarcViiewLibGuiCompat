#include <gccore.h>
#include <wiiuse/wpad.h>
#include <stdint.h>
#include <stdlib.h>

#include "drivers/ogc/OgcVideoDriver.h"
#include "GuiTextRenderer.h"
#include "filelist.h"
#include "drivers/ogc/wii/WiiPlatform.h"

WiiPlatform platformInstance;
Platform* platform = &platformInstance;

int main(int, char **)
{
    WPAD_Init();

    OgcVideoDriver video;
    video.init(640, 480);

    ImageRenderer* image = video.getImageRenderer();
    GlyphRenderer* glyph = video.getGlyphRenderer();

    // Now exercise the real libgui FreeType2 pipeline. The TTF is linked
    // from libgui/data/fonts and remains resident for the renderer lifetime.
    GuiTextRenderer text(font_ttf, font_ttf_size, glyph, 1.0f);
    fontSystem = &text;

    // Test the actual GuiText widget on top of the now-proven renderer.
    GuiText guiText("GuiText works!", 24, {255, 255, 255, 255});
    guiText.setAlignment(ALIGN_H::LEFT, ALIGN_V::TOP);
    guiText.setPosition(80, 340);

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

        // Test the glyph renderer's solid feature path separately.
        glyph->drawFeature(120, 400, 180, 40, {255, 220, 40, 255});

        // Draw the actual GuiText widget, which calls fontSystem internally.
        guiText.draw();

        // Build a tiny synthetic grayscale glyph bitmap. This exercises the
        // FreeType-style FT_Bitmap -> I4 conversion without loading FreeType
        // or any real font yet.
        const uint16_t glyphWidth = 16;
        const uint16_t glyphHeight = 16;
        uint8_t glyphPixels[glyphWidth * glyphHeight];

        for (uint16_t y = 0; y < glyphHeight; ++y) {
            for (uint16_t x = 0; x < glyphWidth; ++x) {
                bool mark = (x == y) || (x + y == glyphWidth - 1);
                glyphPixels[y * glyphWidth + x] = mark ? 255 : 0;
            }
        }

        FT_Bitmap bitmap = {};
        bitmap.width = glyphWidth;
        bitmap.rows = glyphHeight;
        bitmap.pitch = glyphWidth;
        bitmap.buffer = glyphPixels;

        void* glyphTexture = glyph->createTexture(glyphWidth, glyphHeight);
        glyph->loadTextureData(glyphTexture, &bitmap);
        glyph->drawQuad(glyphTexture, 340, 390, glyphWidth, glyphHeight,
                        {255, 255, 255, 255});
        glyph->destroyTexture(glyphTexture);

        video.render();
    }

    image->destroyTexture(texture);

    return 0;
}
