/****************************************************************************
 * MarcViiewLibGuiTest
 * Minimal libgui integration test for Wii.
 *
 * This intentionally contains no MarcViiew/NAND/ViiewLib code.
 ***************************************************************************/

#include <cstdlib>

#include "drivers/Platform.h"
#include "drivers/InputController.h"
#include "libgui/Gui.h"

#include "font_ttf.h"
#include "en_lang.h"

static WiiPlatform platformInstance;
Platform* platform = &platformInstance;

int main(int, char **)
{
    PlatformConfig config;
    config.canvasWidth = 640;
    config.canvasHeight = 480;

    platform->init(config);

    const size_t imageDecodeScratchSize =
        (640 * 480 * 4) + (480 * sizeof(void *));

    GuiImageData::setDecodeScratch(
        std::malloc(imageDecodeScratchSize),
        imageDecodeScratchSize);

    fontSystem = new GuiTextRenderer(
        font_ttf,
        font_ttf_size,
        platform->getVideo()->getGlyphRenderer(),
        platform->getVideo()->getUIScale());

    textTranslator = new GuiTextTranslator();
    textTranslator->loadLanguage(en_lang, en_lang_size);

    platform->getAudio()->start();
    InitUserInputControllers();

    GuiWindow mainWindow(
        platform->getVideo()->getScreenWidth(),
        platform->getVideo()->getScreenHeight());

    GuiText title(
        "MarcViiew libgui Test",
        30,
        (PixelColor){255, 255, 255, 255});
    title.setAlignment(ALIGN_H::CENTRE, ALIGN_V::TOP);
    title.setPosition(0, 45);

    GuiText status(
        "libgui initialised successfully",
        22,
        (PixelColor){220, 220, 220, 255});
    status.setAlignment(ALIGN_H::CENTRE, ALIGN_V::TOP);
    status.setPosition(0, 95);

    GuiText testLabel(
        "Press A",
        22,
        (PixelColor){0, 0, 0, 255});

    GuiImage testImage(
        240,
        70,
        (PixelColor){180, 180, 180, 255});
    GuiImage testImageOver(
        240,
        70,
        (PixelColor){230, 230, 230, 255});

    GuiButton testButton(240, 70);
    testButton.setAlignment(ALIGN_H::CENTRE, ALIGN_V::TOP);
    testButton.setPosition(0, 180);
    testButton.setLabel(&testLabel);
    testButton.setImage(&testImage);
    testButton.setImageOver(&testImageOver);

    GuiText exitLabel(
        "Exit",
        22,
        (PixelColor){0, 0, 0, 255});

    GuiImage exitImage(
        240,
        70,
        (PixelColor){180, 180, 180, 255});
    GuiImage exitImageOver(
        240,
        70,
        (PixelColor){230, 230, 230, 255});

    GuiButton exitButton(240, 70);
    exitButton.setAlignment(ALIGN_H::CENTRE, ALIGN_V::TOP);
    exitButton.setPosition(0, 275);
    exitButton.setLabel(&exitLabel);
    exitButton.setImage(&exitImage);
    exitButton.setImageOver(&exitImageOver);

    GuiTrigger primaryTrigger;
    primaryTrigger.setPrimaryTrigger();

    GuiTrigger secondaryTrigger;
    secondaryTrigger.setSecondaryTrigger();

    testButton.setTrigger(&primaryTrigger);
    exitButton.setTrigger(&primaryTrigger);
    exitButton.setTrigger(&secondaryTrigger);

    testButton.setState(STATE::SELECTED);

    mainWindow.append(&title);
    mainWindow.append(&status);
    mainWindow.append(&testButton);
    mainWindow.append(&exitButton);

    while (!platform->shouldExit())
    {
        platform->getInput()->update();

        for (int i = 3; i >= 0; --i)
            mainWindow.update(controller[i]);

        if (testButton.getState() == STATE::CLICKED)
        {
            status.setText("Button input works!");
            testButton.resetState();
        }

        if (exitButton.getState() == STATE::CLICKED)
            platform->triggerExit();

        mainWindow.draw();
        platform->getVideo()->render();
    }

    platform->requestExit();
}
