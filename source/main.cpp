#include <gccore.h>
#include <wiiuse/wpad.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "drivers/ogc/OgcVideoDriver.h"
#include "GuiTextRenderer.h"
#include "filelist.h"
#include "drivers/ogc/wii/WiiPlatform.h"
#include "drivers/ogc/OgcAudioDriver.h"

WiiPlatform platformInstance;
Platform* platform = &platformInstance;

int main(int, char **)
{
    WPAD_Init();

    PlatformConfig config;
    config.canvasWidth = 640;
    config.canvasHeight = 480;
    config.assetScaleX = 1.0f;
    config.assetScaleY = 1.0f;

    // Exercise the real WiiPlatform compatibility composition root through
    // stage 2: thread + video + audio. Input remains directly driven below
    // for this controlled audio test.
    platformInstance.init(config);

    VideoDriver* video = platform->getVideo();
    AudioDriver* audio = platform->getAudio();

    ImageRenderer* image = video->getImageRenderer();
    GlyphRenderer* glyph = video->getGlyphRenderer();

    // Now exercise the real libgui FreeType2 pipeline. The TTF is linked
    // from libgui/data/fonts and remains resident for the renderer lifetime.
    GuiTextRenderer text(font_ttf, font_ttf_size, glyph, 1.0f);
    fontSystem = &text;

    // Exercise several GuiText features now that the basic widget is proven
    // on real Wii hardware: alignment, sizing, colour, scaling, and wrapping.
    class TestContainer : public GuiElement {
    public:
        void draw() override {}
    };

    TestContainer textPanel;
    textPanel.setPosition(60, 300);
    textPanel.setSize(520, 150);

    GuiText leftText("Left aligned", 20, {255, 255, 255, 255});
    leftText.setParent(&textPanel);
    leftText.setSize(0, 0);
    leftText.setAlignment(ALIGN_H::LEFT, ALIGN_V::TOP);

    GuiText centreText("Centre aligned", 24, {80, 220, 255, 255});
    centreText.setParent(&textPanel);
    centreText.setSize(0, 0);
    centreText.setAlignment(ALIGN_H::CENTRE, ALIGN_V::MIDDLE);
    centreText.setPosition(0, 38);

    GuiText rightText("Right aligned", 20, {255, 220, 80, 255});
    rightText.setParent(&textPanel);
    rightText.setSize(0, 0);
    rightText.setAlignment(ALIGN_H::RIGHT, ALIGN_V::TOP);
    rightText.setPosition(0, 88);

    GuiText wrappedText("Wrapping is working too!", 18, {180, 255, 180, 255});
    wrappedText.setParent(&textPanel);
    wrappedText.setSize(220, 50);
    wrappedText.setAlignment(ALIGN_H::LEFT, ALIGN_V::TOP);
    wrappedText.setPosition(0, 112);
    wrappedText.setWrap(true, 220);

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
    // Audio init is the isolated stage-2 test. Do not submit a voice yet.

    while (SYS_MainLoop())
    {
        WPAD_ScanPads();

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;

        video->clearScreen({40, 40, 40, 255});

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

        // Draw the GuiText feature suite.
        leftText.draw();
        centreText.draw();
        rightText.draw();
        wrappedText.draw();

        video->render();
    }

    image->destroyTexture(texture);
    // WiiPlatform owns the audio driver; leave shutdown to platform teardown.

    return 0;
}
