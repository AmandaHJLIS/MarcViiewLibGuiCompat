#include <gccore.h>
#include <wiiuse/wpad.h>
#include <asndlib.h>
#include <ogc/dsp.h>
#include <malloc.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

int main(int, char **)
{
    WPAD_Init();

    // Raw legacy libogc diagnostic: deliberately bypass libgui's AudioDriver.
    // This tells us whether the failure is inside ASND/libogc itself or in
    // the libgui compatibility layer.
    DSP_Unhalt();
    ASND_Init();
    ASND_Pause(0);

    const int frames = 4800; // 100 ms at 48 kHz
    const int bytes = frames * 2 * (int)sizeof(int16_t);
    int16_t *tone = (int16_t *)memalign(32, bytes);
    if (!tone)
        return 1;

    memset(tone, 0, bytes);
    for (int i = 0; i < frames; ++i)
    {
        int16_t sample = (int16_t)(12000.0f *
            sinf(2.0f * 3.14159265f * 440.0f * i / 48000.0f));
        tone[i * 2] = sample;
        tone[i * 2 + 1] = sample;
    }

    s32 voice = ASND_GetFirstUnusedVoice();
    s32 result = -1;
    if (voice >= 0)
        result = ASND_SetVoice(
            voice,
            VOICE_STEREO_16BIT,
            48000,
            0,
            tone,
            bytes,
            110,
            110,
            NULL);

    // Keep the buffer alive while ASND is using it.
    while (SYS_MainLoop())
    {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;
    }

    if (voice >= 0 && result == SND_OK)
        ASND_StopVoice(voice);

    ASND_Pause(1);
    ASND_End();
    free(tone);
    return 0;
}
