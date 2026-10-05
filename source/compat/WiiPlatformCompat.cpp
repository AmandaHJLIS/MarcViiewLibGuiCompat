/****************************************************************************
 * MarcViiewLibGuiCompat - legacy Wii startup compatibility
 * Staged WiiPlatform initialisation for real-hardware diagnostics.
 *
 * The upstream WiiPlatform::init() starts every subsystem in one call:
 * thread, video, audio, input and Wii filesystem/USB/DVD. That makes a
 * hardware black-screen impossible to localise.
 *
 * LIBGUI_COMPAT_STAGE selects how far startup proceeds:
 *   1 = thread + video
 *   2 = + audio
 *   3 = + input
 *   4 = + filesystem
 *   5 = full startup
 ***************************************************************************/
#include "WiiPlatform.h"

void WiiPlatform::init(const PlatformConfig& config)
{
    this->config = config;

    this->threadDriver = new OgcThreadDriver();
    this->threadDriver->init();

    this->videoDriver = new OgcVideoDriver();
    this->videoDriver->init(config.canvasWidth, config.canvasHeight);

#if LIBGUI_COMPAT_STAGE >= 2
    this->audioDriver = new OgcAudioDriver();
    this->audioDriver->init();
#endif

#if LIBGUI_COMPAT_STAGE >= 3
    this->inputDriver = new OgcInputDriver();
    this->inputDriver->init();
#endif

#if LIBGUI_COMPAT_STAGE >= 4
    this->fileSystemDriver = new WiiFileSystemDriver();
    this->fileSystemDriver->init();
#endif

#if LOGGING_ENABLED
    if (LIBGUI_COMPAT_STAGE >= 5)
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
