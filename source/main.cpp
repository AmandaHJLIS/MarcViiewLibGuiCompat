/****************************************************************************
 * MarcViiewLibGuiCompat - raw legacy libogc video/GX diagnostic
 *
 * This deliberately bypasses libgui, WiiPlatform, and all compatibility
 * drivers. It tests only the legacy libogc VIDEO/GX path on real hardware.
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_FIFO_SIZE (256 * 1024)

static void *frameBuffer[2] = { nullptr, nullptr };
static GXRModeObj *rmode = nullptr;
static void *gp_fifo = nullptr;
static u32 fb = 0;

static void drawFrame()
{
    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);

    GX_SetCopyClear((GXColor){40, 40, 40, 255}, GX_MAX_Z24);
    GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

    GX_Position3f32(-0.5f, -0.5f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);

    GX_Position3f32( 0.5f, -0.5f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);

    GX_Position3f32( 0.5f,  0.5f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);

    GX_Position3f32(-0.5f,  0.5f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);

    GX_End();

    GX_DrawDone();
    GX_CopyDisp(frameBuffer[fb], GX_TRUE);

    VIDEO_SetNextFramebuffer(frameBuffer[fb]);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    fb ^= 1;
}

int main(int, char **)
{
    VIDEO_Init();

    rmode = VIDEO_GetPreferredMode(nullptr);

    frameBuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    frameBuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(frameBuffer[fb]);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    gp_fifo = memalign(32, DEFAULT_FIFO_SIZE);
    memset(gp_fifo, 0, DEFAULT_FIFO_SIZE);
    GX_Init(gp_fifo, DEFAULT_FIFO_SIZE);

    GX_SetCullMode(GX_CULL_NONE);
    GX_SetDispCopyGamma(GX_GM_1_0);

    while (true)
        drawFrame();

    return 0;
}
