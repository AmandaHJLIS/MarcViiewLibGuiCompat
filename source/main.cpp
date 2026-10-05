#include <gccore.h>
#include <stdint.h>
#include <stdio.h>

#include "drivers/ogc/wii/WiiPlatform.h"
#include "drivers/InputController.h"
#include "GuiTextRenderer.h"
#include "filelist.h"

WiiPlatform platformInstance;
Platform* platform = &platformInstance;

int main(int, char **)
{
    PlatformConfig config;
    config.canvasWidth = 640;
    config.canvasHeight = 480;

    platform->init(config);

    GlyphRenderer* glyph = platform->getVideo()->getGlyphRenderer();

    GuiTextRenderer text(font_ttf, font_ttf_size, glyph, 1.0f);
    fontSystem = &text;

    GuiText title("libgui input compatibility", 24, {255, 255, 255, 255});
    title.setPosition(40, 35);
    title.setSize(560, 40);
    title.setAlignment(ALIGN_H::CENTRE, ALIGN_V::TOP);

    GuiText instructions(
        "Press A / B / D-pad / HOME and point the Wii Remote",
        16,
        {190, 220, 255, 255});
    instructions.setPosition(40, 78);
    instructions.setSize(560, 40);
    instructions.setAlignment(ALIGN_H::CENTRE, ALIGN_V::TOP);

    GuiText statusText("Waiting for input...", 18, {180, 255, 180, 255});
    statusText.setPosition(60, 170);
    statusText.setSize(520, 50);
    statusText.setAlignment(ALIGN_H::CENTRE, ALIGN_V::MIDDLE);

    GuiText pointerText("Pointer: inactive", 18, {255, 220, 120, 255});
    pointerText.setPosition(60, 235);
    pointerText.setSize(520, 40);
    pointerText.setAlignment(ALIGN_H::CENTRE, ALIGN_V::MIDDLE);

    GuiText heldText("Held buttons: none", 16, {210, 210, 210, 255});
    heldText.setPosition(60, 285);
    heldText.setSize(520, 40);
    heldText.setAlignment(ALIGN_H::CENTRE, ALIGN_V::MIDDLE);

    GuiText orientationText("Orientation: normal", 16, {210, 210, 210, 255});
    orientationText.setPosition(60, 325);
    orientationText.setSize(520, 40);
    orientationText.setAlignment(ALIGN_H::CENTRE, ALIGN_V::MIDDLE);

    GuiText connectionText("Controller 1: disconnected", 16, {210, 210, 210, 255});
    connectionText.setPosition(60, 365);
    connectionText.setSize(520, 40);
    connectionText.setAlignment(ALIGN_H::CENTRE, ALIGN_V::MIDDLE);

    GuiText navigationText("Navigation: none", 16, {210, 210, 210, 255});
    navigationText.setPosition(60, 405);
    navigationText.setSize(520, 40);
    navigationText.setAlignment(ALIGN_H::CENTRE, ALIGN_V::MIDDLE);

    while (SYS_MainLoop())
    {
        platform->getInput()->update();

        InputController* pad = controller[0];
        const InputPadData& data = pad->getPadData();

        if (pad->isPressed(INPUT_BTN_HOME))
            break;

        if (pad->isPressed(INPUT_BTN_A))
            statusText.setText("A pressed");
        else if (pad->isPressed(INPUT_BTN_B))
            statusText.setText("B pressed");
        else if (pad->isPressed(INPUT_BTN_1))
            statusText.setText("1 pressed");
        else if (pad->isPressed(INPUT_BTN_2))
            statusText.setText("2 pressed");
        else if (pad->isPressed(INPUT_BTN_PLUS))
            statusText.setText("PLUS pressed");
        else if (pad->isPressed(INPUT_BTN_MINUS))
            statusText.setText("MINUS pressed");

        char held[96];
        snprintf(
            held, sizeof(held),
            "Held: %s%s%s%s",
            pad->isHeld(INPUT_BTN_A) ? "A " : "",
            pad->isHeld(INPUT_BTN_B) ? "B " : "",
            pad->isHeld(INPUT_BTN_UP) ? "UP " : "",
            pad->isHeld(INPUT_BTN_DOWN) ? "DOWN " : "");
        heldText.setText(held);

        if (data.validPointer)
        {
            char pointer[96];
            snprintf(
                pointer, sizeof(pointer),
                "Pointer: active  X=%d  Y=%d",
                (int)data.cursor_x, (int)data.cursor_y);
            pointerText.setText(pointer);
        }
        else
        {
            pointerText.setText("Pointer: inactive");
        }

        orientationText.setText(
            pad->isSideways()
                ? "Orientation: sideways"
                : "Orientation: normal");

        connectionText.setText(
            data.hw_connected[INPUT_HW_WIIMOTE]
                ? "Controller 1: Wii Remote connected"
                : "Controller 1: disconnected");

        if (pad->up())
            navigationText.setText("Navigation: UP");
        else if (pad->down())
            navigationText.setText("Navigation: DOWN");
        else if (pad->left())
            navigationText.setText("Navigation: LEFT");
        else if (pad->right())
            navigationText.setText("Navigation: RIGHT");
        else
            navigationText.setText("Navigation: none");

        platform->getVideo()->clearScreen({40, 40, 40, 255});

        title.draw();
        instructions.draw();
        statusText.draw();
        pointerText.draw();
        heldText.draw();
        orientationText.draw();
        connectionText.draw();
        navigationText.draw();

        platform->getVideo()->render();
    }

    platform->requestExit();
    return 0;
}
