/****************************************************************************
 * MarcViiewLibGuiCompat - isolated OgcVideoDriver diagnostic
 *
 * This test adds exactly one compatibility layer on top of the previously
 * proven raw VIDEO/GX path: upstream libgui's OgcVideoDriver interface,
 * implemented by OgcVideoDriverCompat.cpp for legacy libogc.
 ***************************************************************************/

#include <stdlib.h>\n#include <gccore.h>
#include <wiiuse/wpad.h>

#include "drivers/ogc/OgcVideoDriver.h"

int main(int, char **)
{
    WPAD_Init();

    OgcVideoDriver video;
    video.init(640, 480);

    // Establish a minimal non-textured GX pipeline after driver initialization.
    GX_SetViewport(0, 0, 640, 480, 0, 1);
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetNumChans(1);
    GX_SetNumTexGens(0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);

    Mtx44 projection;
    guOrtho(projection, 0, 480, 0, 640, 0, 300);
    GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);

    Mtx modelView;
    guMtxIdentity(modelView);
    GX_LoadPosMtxImm(modelView, GX_PNMTX0);
    GX_SetCurrentMtx(GX_PNMTX0);
    video.clearScreen({40, 40, 40, 255});

    while (SYS_MainLoop())
    {
        WPAD_ScanPads();

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;

        // Draw a deliberately simple quad using the GX state initialized by
        // OgcVideoDriver. The driver itself does not draw scene geometry;
        // libgui normally does that through its renderers.
        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(0.0f, 0.0f, 0.0f);
        GX_Color4u8(40, 40, 40, 255);
        GX_Position3f32(640.0f, 0.0f, 0.0f);
        GX_Color4u8(40, 40, 40, 255);
        GX_Position3f32(640.0f, 480.0f, 0.0f);
        GX_Color4u8(40, 40, 40, 255);
        GX_Position3f32(0.0f, 480.0f, 0.0f);
        GX_Color4u8(40, 40, 40, 255);
        GX_End();

        video.render();
    }

    // Do not manually tear down the video driver here. Returning from main()
    // lets libogc perform its normal loader/reset cleanup after SYS_MainLoop()
    // requests exit. Manual GX/VI teardown was producing corrupted video when
    // returning to the Homebrew Channel on real hardware.
    return 0;
}
