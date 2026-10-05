/****************************************************************************
 * MarcViiewLibGuiCompat - raw legacy libogc video/GX + WPAD diagnostic
 *
 * Deliberately bypasses libgui and every compatibility driver. This tests
 * the known-good VIDEO/GX path, then adds only Wii Remote input and a clean
 * exit path.
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>

#define DEFAULT_FIFO_SIZE (256 * 1024)

static void *frameBuffer[2] = { nullptr, nullptr };
static GXRModeObj *rmode = nullptr;
static void *gp_fifo = nullptr;
static u32 fb = 0;

static void drawFrame()
{
    Mtx44 projection;

    guOrtho(projection,
            -1.0f, 1.0f,
            -1.0f, 1.0f,
            -1.0f, 1.0f);

    GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);

    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);

    GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GX_SetColorUpdate(GX_TRUE);

    GX_SetNumChans(1);
    GX_SetNumTexGens(0);
    GX_SetTevOrder(GX_TEVSTAGE0,
                   GX_TEXCOORDNULL,
                   GX_TEXMAP_NULL,
                   GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,
                     GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0,
                     GX_CLR_RGBA, GX_RGBA8, 0);

    GX_SetCopyClear((GXColor){40, 40, 40, 255}, 0x00ffffff);

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

    fb ^= 1;
}

int main(int, char **)
{
    VIDEO_Init();
    WPAD_Init();

    rmode = VIDEO_GetPreferredMode(nullptr);

    gp_fifo = memalign(32, DEFAULT_FIFO_SIZE);
    memset(gp_fifo, 0, DEFAULT_FIFO_SIZE);

    frameBuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    frameBuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(frameBuffer[fb]);
    VIDEO_SetBlack(false);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    GX_Init(gp_fifo, DEFAULT_FIFO_SIZE);

    GX_SetCopyClear((GXColor){40, 40, 40, 255}, 0x00ffffff);
    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);

    const f32 yscale =
        GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    const u32 xfbHeight = GX_SetDispCopyYScale(yscale);

    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);

    GX_SetCopyFilter(rmode->aa,
                     rmode->sample_pattern,
                     GX_TRUE,
                     rmode->vfilter);

    GX_SetFieldMode(rmode->field_rendering,
                    (rmode->viHeight == 2 * rmode->xfbHeight)
                        ? GX_ENABLE
                        : GX_DISABLE);

    if (rmode->aa)
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    else
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

    GX_SetCullMode(GX_CULL_NONE);
    GX_CopyDisp(frameBuffer[fb], GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);

    while (SYS_MainLoop())
    {
        WPAD_ScanPads();

        const u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_HOME)
            exit(0);

        drawFrame();
    }

    return 0;
}
