/****************************************************************************
 * MarcViiewLibGuiCompat - legacy Wii startup compatibility
 * Staged WiiPlatform initialisation for real-hardware diagnostics.
 *
 * The upstream WiiPlatform::init() starts every subsystem in one call:
 * thread, video, audio, input and Wii filesystem/USB/DVD. This compatibility
 * implementation deliberately skips audio because the current hardware test
 * does not exercise libgui audio.
 *
 * LIBGUI_COMPAT_STAGE selects how far startup proceeds:
 *   1 = thread + video
 *   2 = + input
 *   3 = + filesystem
 *   4 = full startup
 ***************************************************************************/
#include <gccore.h>

#include "drivers/ogc/wii/WiiPlatform.h"

static void showStartupCheckpoint(VideoDriver *video, const PixelColor& color)
{
    if (!video)
        return;

    for (int i = 0; i < 180 && SYS_MainLoop(); ++i)
    {
        video->clearScreen(color);
        video->render();
    }
}

void WiiPlatform::init(const PlatformConfig& config)
{
    this->config = config;

    this->threadDriver = new OgcThreadDriver();
    this->threadDriver->init();

    this->videoDriver = new OgcVideoDriver();
    this->videoDriver->init(config.canvasWidth, config.canvasHeight);

    // If we never see red, the problem is before/inside video setup.
    showStartupCheckpoint(this->videoDriver, {255, 0, 0, 255});

#if LIBGUI_COMPAT_STAGE >= 2
    this->inputDriver = new OgcInputDriver();
    this->inputDriver->init();

    // If red works but green does not, input startup is the suspect.
    showStartupCheckpoint(this->videoDriver, {0, 255, 0, 255});
#endif

#if LIBGUI_COMPAT_STAGE >= 3
    this->fileSystemDriver = new WiiFileSystemDriver();
    this->fileSystemDriver->init();
#endif

#if LOGGING_ENABLED
    if (LIBGUI_COMPAT_STAGE >= 4)
    {
        this->logger = new Logger();
        this->logger->registerBackend(LOGGER_OSREPORT, new OgcLoggerSysReport());
        this->logger->registerBackend(LOGGER_UDP, new OgcLoggerUdp());
        this->logger->registerBackend(LOGGER_SERIAL, new OgcLoggerUsbGecko());
        this->logger->registerBackend(LOGGER_FILE, new LoggerFile());

        LogConfig logConfig;
        static const int deviceCandidates[] = { DEVICE_SD, DEVICE_USB };

        if (this->fileSystemDriver)
        {
            const char * mountPath =
                FindFirstMountedPath(this->fileSystemDriver, deviceCandidates, 2);

            if (mountPath[0] != '\0')
            {
                snprintf(
                    logConfig.filePath,
                    sizeof(logConfig.filePath),
                    "%sdebug.log",
                    mountPath);
            }
        }

        this->logger->init(logConfig);
    }
#endif
}

void WiiPlatform::shutdown()
{
    if (logger)
    {
        logger->shutdown();
        delete logger;
        logger = nullptr;
    }

    if (fileSystemDriver)
    {
        fileSystemDriver->shutdown();
        delete fileSystemDriver;
        fileSystemDriver = nullptr;
    }

    if (inputDriver)
    {
        inputDriver->shutdown();
        delete inputDriver;
        inputDriver = nullptr;
    }

    if (audioDriver)
    {
        audioDriver->shutdown();
        delete audioDriver;
        audioDriver = nullptr;
    }

    if (videoDriver)
    {
        videoDriver->shutdown();
        delete videoDriver;
        videoDriver = nullptr;
    }

    if (threadDriver)
    {
        threadDriver->shutdown();
        delete threadDriver;
        threadDriver = nullptr;
    }
}

/****************************************************************************
 * Shutdown/reset
 ***************************************************************************/

static bool hardwarePowerOffRequested = false;

void NotifyWiiShutdownRequested()
{
    hardwarePowerOffRequested = true;
    platform->triggerExit();
}

void WiiPlatform::requestExit()
{
    this->shutdown();

    if (hardwarePowerOffRequested)
        SYS_ResetSystem(SYS_POWEROFF_STANDBY, 0, FALSE);
    else
        exit(0);
}

SystemEvent WiiPlatform::getSystemEvent()
{
    if (platform->getStatus() == Status::Exiting)
        return SystemEvent::ShutdownRequested;

    static bool wasResetDown = false;
    bool isResetDown = SYS_ResetButtonDown();
    bool justPressed = isResetDown && !wasResetDown;
    wasResetDown = isResetDown;

    if (justPressed)
        return SystemEvent::ResetRequested;

    return SystemEvent::None;
}
