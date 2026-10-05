#include <gccore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "drivers/ogc/wii/WiiPlatform.h"
#include "drivers/InputController.h"
#include "GuiButton.h"
#include "GuiTrigger.h"
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

    ImageRenderer* image = platform->getVideo()->getImageRenderer();
    GlyphRenderer* glyph = platform->getVideo()->getGlyphRenderer();

    GuiTextRenderer text(font_ttf, font_ttf_size, glyph, 1.0f);
    fontSystem = &text;

    class TestContainer : public GuiElement {
    public:
        void draw() override {}
    };

    TestContainer textPanel;
    textPanel.setPosition(40, 35);
    textPanel.setSize(560, 90);

    GuiText title("libgui input compatibility", 24, {255, 255, 255, 255});
    title.setParent(&textPanel);
    title.setSize(560, 40);
    title.setAlignment(ALIGN_H::CENTRE, ALIGN_V::TOP);

    GuiText instructions(
        "Point at the button and press A, or press HOME to exit",
        16,
        {190, 220, 255, 255});
    instructions.setParent(&textPanel);
    instructions.setSize(560, 40);
    instructions.setAlignment(ALIGN_H::CENTRE, ALIGN_V::TOP);
    instructions.setPosition(0, 42);

    GuiButton testButton(300, 70);
    testButton.setPosition(170, 170);

    GuiText buttonLabel("PRESS A", 24, {255, 255, 255, 255});
    buttonLabel.setSize(300, 70);
    buttonLabel.setAlignment(ALIGN_H::CENTRE, ALIGN_V::MIDDLE);
    testButton.setLabel(&buttonLabel);

    // BUTTON_ONLY deliberately exercises the real GuiTrigger -> InputController
    // path without requiring a separate GuiWindow/focus implementation.
    GuiTrigger triggerA;
    triggerA.setButtonOnlyTrigger(0, INPUT_BTN_A);
    testButton.setTrigger(&triggerA);

    GuiText statusText("Waiting for input...", 18, {180, 255, 180, 255});
    statusText.setPosition(80, 285);
    statusText.setSize(480, 50);
    statusText.setAlignment(ALIGN_H::CENTRE, ALIGN_V::MIDDLE);

    GuiText pointerText("Pointer: inactive", 16, {255, 220, 120, 255});
    pointerText.setPosition(80, 345);
    pointerText.setSize(480, 40);
    pointerText.setAlignment(ALIGN_H::CENTRE, ALIGN_V::MIDDLE);

    GuiText buttonStateText("Button state: DEFAULT", 16, {200, 200, 200, 255});
    buttonStateText.setPosition(80, 390);
    buttonStateText.setSize(480, 40);
    buttonStateText.setAlignment(ALIGN_H::CENTRE, ALIGN_V::MIDDLE);

    int clickCount = 0;
    char clickCountText[64];
    GuiText countText("Clicks: 0", 16, {180, 255, 180, 255});
    countText.setPosition(80, 435);
    countText.setSize(480, 30);
    countText.setAlignment(ALIGN_H::CENTRE, ALIGN_V::TOP);

    while (SYS_MainLoop())
    {
        platform->getInput()->update();

        InputController* pad = controller[0];

        if (pad->isPressed(INPUT_BTN_HOME))
            break;

        testButton.update(pad);

        if (testButton.getState() == STATE::CLICKED)
        {
            ++clickCount;
            statusText.setText("A button click received!");
            testButton.resetState();
        }

        if (pad->getPadData().validPointer)
            pointerText.setText("Pointer: active");
        else
            pointerText.setText("Pointer: inactive");

        if (testButton.getState() == STATE::SELECTED)
            buttonStateText.setText("Button state: SELECTED");
        else
            buttonStateText.setText("Button state: DEFAULT");

        platform->getVideo()->clearScreen({40, 40, 40, 255});

        image->drawRectangle(
            120.0f, 135.0f, 400.0f, 10.0f,
            {50, 120, 180, 255});

        PixelColor buttonColour =
            testButton.getState() == STATE::SELECTED
                ? PixelColor{70, 170, 230, 255}
                : PixelColor{50, 100, 150, 255};

        image->drawRectangle(
            170.0f, 170.0f, 300.0f, 70.0f,
            buttonColour);

        title.draw();
        instructions.draw();
        testButton.draw();
        statusText.draw();
        pointerText.draw();
        buttonStateText.draw();

        snprintf(clickCountText, sizeof(clickCountText), "Clicks: %d", clickCount);
        countText.setText(clickCountText);
        countText.draw();

        platform->getVideo()->render();
    }

    platform->requestExit();
    return 0;
}
