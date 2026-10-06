#include <gccore.h>

#include "drivers/ogc/OgcVideoDriver.h"
#include "drivers/ogc/OgcInputDriver.h"
#include "drivers/Platform.h"
#include "drivers/InputController.h"

static OgcVideoDriver video;
static OgcInputDriver input;

class DiagnosticPlatform : public Platform
{
public:
    void init(const PlatformConfig&) override {}
    void requestExit() override {}
    AudioDriver* getAudio() override { return nullptr; }
    VideoDriver* getVideo() override { return &video; }
    InputDriver* getInput() override { return &input; }
    FileSystemDriver* getFileSystem() override { return nullptr; }
    ThreadDriver* getThread() override { return nullptr; }
    Logger* getLogger() override { return nullptr; }
    SystemEvent getSystemEvent() override { return SystemEvent::None; }
    Status getStatus() const override { return Status::Running; }
    void triggerExit() override {}
protected:
    void shutdown() override {}
};

static DiagnosticPlatform diagnosticPlatform;
Platform* platform = &diagnosticPlatform;

static void showColour(const PixelColor& color, int frames)
{
    for (int i = 0; i < frames && SYS_MainLoop(); ++i) {
        video.clearScreen(color);
        video.render();
    }
}

int main(int, char **)
{
    video.init(640, 480);
    showColour({255, 0, 0, 255}, 180);

    input.init();
    showColour({0, 255, 0, 255}, 180);

    while (SYS_MainLoop()) {
        input.update();

        if (controller[0]->isPressed(INPUT_BTN_HOME))
            break;

        video.clearScreen({0, 0, 255, 255});
        video.render();
    }

    input.shutdown();
    video.shutdown();
    return 0;
}
