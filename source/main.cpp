#include <gccore.h>
#include <wiiuse/wpad.h>

#include "drivers/ogc/wii/WiiPlatform.h"
#include "libgui/Gui.h"
#include "font_ttf.h"

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

    text.setPosition(0, 200);

    showColour({255, 255, 0, 255}, 120);

    // This is the initialization performed by libgui's normal demo
    // application before any GuiText is drawn.
    fontSystem = new GuiTextRenderer(
        font_ttf,
        font_ttf_size,
        platform->getVideo()->getGlyphRenderer(),
        platform->getVideo()->getUIScale());

    // Confirm that the renderer was constructed before attempting draw().
    if (fontSystem == nullptr)
        showColour({255, 0, 255, 255}, 180);
    else
        showColour({0, 255, 255, 255}, 180);

    while (SYS_MainLoop())
    {
        if (homePressed())
            break;

        platform->getVideo()->clearScreen({0, 0, 0, 255});

        // Diagnostic: draw a solid GX quad through the glyph renderer,
        // bypassing FreeType and glyph textures entirely.
        platform->getVideo()->getGlyphRenderer()->drawFeature(
            0, 200, 300, 60, {255, 255, 255, 255});

        text.draw();
        platform->getVideo()->render();
    }

    delete fontSystem;
    fontSystem = nullptr;

    return 0;
}
