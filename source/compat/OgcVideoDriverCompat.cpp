/****************************************************************************
 * MarcViiewLibGuiCompat - legacy libogc OgcVideoDriver compatibility
 *
 * This is an isolated compatibility implementation of upstream libgui's
 * OgcVideoDriver. It deliberately contains only the video/GX layer; audio,
 * input, filesystem, threading, and WiiPlatform are not involved.
 *
 * The implementation is adapted from upstream dborth/libgui's
 * source/drivers/ogc/OgcVideoDriver.cpp, with legacy libogc-compatible
 * framebuffer handling and VIDEO_WaitVSync().
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <ogc/machine/processor.h>

#include "OgcVideoDriver.h"
#include "../../libgui/Gui.h"

#define DEFAULT_FIFO_SIZE (256 * 1024)

static Mtx GXmodelView2D;

OgcVideoDriver::OgcVideoDriver()
    : imageRenderer(nullptr), glyphRenderer(nullptr)
{
}

OgcVideoDriver::~OgcVideoDriver()
{
    delete imageRenderer;
    delete glyphRenderer;
}

void OgcVideoDriver::init(int width, int height)
{
    VIDEO_Init();

#ifdef HW_RVL
    if (CONF_GetAspectRatio() == CONF_ASPECT_16_9 &&
        (*(u32*)(0xCD8005A0) >> 16) == 0xCAFE)
    {
        write32(0xd8006a0, 0x30000004);
        mask32(0xd8006a8, 0, 2);
    }
#endif

    vmode = VIDEO_GetPreferredMode(nullptr);

#ifdef HW_RVL
    if (CONF_GetAspectRatio() == CONF_ASPECT_16_9)
        vmode->viWidth = 678;
    else
        vmode->viWidth = 672;

    if ((vmode->viTVMode >> 2) == VI_NTSC)
    {
        vmode->viXOrigin = (VI_MAX_WIDTH_NTSC - vmode->viWidth) / 2;
        vmode->viYOrigin = (VI_MAX_HEIGHT_NTSC - vmode->viHeight) / 2;
    }
    else
    {
        vmode->viXOrigin = (VI_MAX_WIDTH_PAL - vmode->viWidth) / 2;
        vmode->viYOrigin = (VI_MAX_HEIGHT_PAL - vmode->viHeight) / 2;
    }
#endif

    VIDEO_Configure(vmode);

    xfb[0] = (uint32_t*)SYS_AllocateFramebuffer(vmode);
    xfb[1] = (uint32_t*)SYS_AllocateFramebuffer(vmode);

    DCInvalidateRange(xfb[0], VIDEO_GetFrameBufferSize(vmode));
    DCInvalidateRange(xfb[1], VIDEO_GetFrameBufferSize(vmode));

    xfb[0] = (uint32_t*)MEM_K0_TO_K1(xfb[0]);
    xfb[1] = (uint32_t*)MEM_K0_TO_K1(xfb[1]);

    VIDEO_ClearFrameBuffer(vmode, xfb[0], COLOR_BLACK);
    VIDEO_ClearFrameBuffer(vmode, xfb[1], COLOR_BLACK);
    VIDEO_SetNextFramebuffer(xfb[0]);

    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (vmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    screenWidth = width;
    screenHeight = height;
    whichfb = 0;
    frameTimer = 0;

    GXColor background = {0, 0, 0, 0xff};

    gp_fifo = memalign(32, DEFAULT_FIFO_SIZE);
    memset(gp_fifo, 0, DEFAULT_FIFO_SIZE);

    GX_Init(gp_fifo, DEFAULT_FIFO_SIZE);
    GX_SetCopyClear(background, GX_MAX_Z24);
    GX_SetDispCopyGamma(GX_GM_1_0);
    GX_SetCullMode(GX_CULL_NONE);

    resetVideoMenu();

    imageRenderer = new OgcImageRenderer();
    glyphRenderer = new OgcGlyphRenderer();
}

void OgcVideoDriver::shutdown()
{
    GX_AbortFrame();
    GX_Flush();

    VIDEO_SetBlack(TRUE);
    VIDEO_Flush();
}

void OgcVideoDriver::render()
{
    whichfb ^= 1;

    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp(xfb[whichfb], GX_TRUE);
    GX_DrawDone();

    VIDEO_SetNextFramebuffer(xfb[whichfb]);
    VIDEO_Flush();

    // Legacy libogc does not provide libgui's VIDEO_WaitForFlush().
    VIDEO_WaitVSync();

    ++frameTimer;
}

void OgcVideoDriver::clearScreen(const PixelColor& color)
{
    GXColor background = {color.r, color.g, color.b, color.a};
    GX_SetCopyClear(background, GX_MAX_Z24);
}

void OgcVideoDriver::resetVideoMenu()
{
    Mtx44 p;
    float yscale;
    uint32_t xfbHeight;

    GXColor background = {0, 0, 0, 255};
    GX_SetCopyClear(background, GX_MAX_Z24);

    yscale = GX_GetYScaleFactor(vmode->efbHeight, vmode->xfbHeight);
    xfbHeight = GX_SetDispCopyYScale(yscale);

    GX_SetScissor(0, 0, vmode->fbWidth, vmode->efbHeight);
    GX_SetDispCopySrc(0, 0, vmode->fbWidth, vmode->efbHeight);
    GX_SetDispCopyDst(vmode->fbWidth, xfbHeight);

    GX_SetCopyFilter(vmode->aa,
                     vmode->sample_pattern,
                     GX_TRUE,
                     vmode->vfilter);

    GX_SetFieldMode(vmode->field_rendering,
                    ((vmode->viHeight == 2 * vmode->xfbHeight)
                        ? GX_ENABLE
                        : GX_DISABLE));

    if (vmode->aa)
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    else
        GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

    GX_ClearVtxDesc();
    GX_InvVtxCache();
    GX_InvalidateTexAll();

    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_TRUE);

    GX_SetNumChans(1);
    GX_SetNumTexGens(1);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetTevOrder(GX_TEVSTAGE0,
                   GX_TEXCOORD0,
                   GX_TEXMAP0,
                   GX_COLOR0A0);

    GX_SetTexCoordGen(GX_TEXCOORD0,
                      GX_TG_MTX2x4,
                      GX_TG_TEX0,
                      GX_IDENTITY);

    guMtxIdentity(GXmodelView2D);
    guMtxTransApply(GXmodelView2D,
                    GXmodelView2D,
                    0.0F,
                    0.0F,
                    -50.0F);

    GX_LoadPosMtxImm(GXmodelView2D, GX_PNMTX0);

    guOrtho(p,
            0,
            screenHeight - 1,
            0,
            screenWidth - 1,
            0,
            300);

    GX_LoadProjectionMtx(p, GX_ORTHOGRAPHIC);

    GX_SetViewport(0,
                   0,
                   vmode->fbWidth,
                   vmode->efbHeight,
                   0,
                   1);

    GX_SetBlendMode(GX_BM_BLEND,
                    GX_BL_SRCALPHA,
                    GX_BL_INVSRCALPHA,
                    GX_LO_CLEAR);

    GX_SetAlphaUpdate(GX_TRUE);
}
