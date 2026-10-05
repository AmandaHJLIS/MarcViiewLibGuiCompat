#include <gccore.h>
#include <wiiuse/wpad.h>
#include <asndlib.h>
#include <ogc/dsp.h>
#include <malloc.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

static void video_init()
{
    VIDEO_Init();
    GXRModeObj *vmode = VIDEO_GetPreferredMode(NULL);
    void *xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
    VIDEO_Configure(vmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    void *fifo = memalign(32, 256 * 1024);
    memset(fifo, 0, 256 * 1024);
    GX_Init(fifo, 256 * 1024);

    GX_SetCopyClear((GXColor){40,40,40,255}, 0x00ffffff);
    GX_SetViewport(0, 0, vmode->fbWidth, vmode->efbHeight, 0, 1);
    GX_SetScissor(0, 0, vmode->fbWidth, vmode->efbHeight);
    GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_FALSE);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GX_SetNumChans(1);
    GX_SetNumTexGens(0);
    GX_SetNumTevStages(1);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
}

static void frame()
{
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position2f32(20,20); GX_Color4u8(40,200,40,255);
    GX_Position2f32(620,20); GX_Color4u8(40,200,40,255);
    GX_Position2f32(620,460); GX_Color4u8(40,200,40,255);
    GX_Position2f32(20,460); GX_Color4u8(40,200,40,255);
    GX_End();
    GX_DrawDone();
    GX_CopyDisp(VIDEO_GetCurrentFramebuffer(), GX_TRUE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
}

int main(int, char **)
{
    WPAD_Init();
    video_init();

    // Establish that the raw video/main-loop path is alive before touching ASND.
    while (SYS_MainLoop())
    {
        WPAD_ScanPads();
        frame();
        break;
    }

    // Stage 1: test ASND_Init() by itself. If Dolphin still throws
    // 01fe01fe here, the failure is inside DSP/ASND initialisation.
    ASND_Init();

    // Blue = ASND_Init() returned to the CPU successfully.
    GX_SetCopyClear((GXColor){40, 100, 220, 255}, 0x00ffffff);
    GX_DrawDone();
    GX_CopyDisp(VIDEO_GetCurrentFramebuffer(), GX_TRUE);
    VIDEO_Flush();

    // Keep the stage result visible for a few seconds.
    for (int i = 0; i < 180; ++i)
        VIDEO_WaitVSync();

    ASND_End();
    return 0;

    const int frames = 4800;
    const int bytes = frames * 2 * (int)sizeof(int16_t);
    int16_t *tone = (int16_t *)memalign(32, bytes);
    if (!tone) return 1;

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
        result = ASND_SetVoice(voice, VOICE_STEREO_16BIT, 48000, 0,
                               tone, bytes, 110, 110, NULL);

    // The result is deliberately rendered without depending on Dolphin's
    // exception dialog. This lets us distinguish ASND_Init() reaching the
    // caller from ASND_SetVoice() failing afterwards.
    if (result == SND_OK) {
        // Green = ASND_SetVoice succeeded.
        GX_SetCopyClear((GXColor){40, 200, 40, 255}, 0x00ffffff);
    } else if (voice < 0) {
        // Red = no unused ASND voice was available.
        GX_SetCopyClear((GXColor){220, 40, 40, 255}, 0x00ffffff);
    } else {
        // Yellow = ASND initialised, but ASND_SetVoice returned an error.
        GX_SetCopyClear((GXColor){220, 200, 40, 255}, 0x00ffffff);
    }
    GX_DrawDone();
    GX_CopyDisp(VIDEO_GetCurrentFramebuffer(), GX_TRUE);
    VIDEO_Flush();

    // Leave the result visible long enough to identify the stage before HBC.
    for (int i = 0; i < 120; ++i)
        VIDEO_WaitVSync();

    if (voice >= 0 && result == SND_OK)
        ASND_StopVoice(voice);
    ASND_Pause(1);
    ASND_End();
    free(tone);
    return 0;
}
