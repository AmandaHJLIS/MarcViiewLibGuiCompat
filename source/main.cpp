#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#include "drivers/ogc/OgcVideoDriver.h"

int main(int, char **)
{
    WPAD_Init();

    OgcVideoDriver video;
    video.init(640, 480);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGB8, 0);

    GX_SetNumChans(1);
    GX_SetNumTexGens(0);
    GX_SetTevOrder(GX_TEVSTAGE0,
                   GX_TEXCOORDNULL,
                   GX_TEXMAP_NULL,
                   GX_COLOR0A0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    Mtx44 perspective;
    guPerspective(perspective, 45.0f, 640.0f / 480.0f, 0.1f, 300.0f);
    GX_LoadProjectionMtx(perspective, GX_PERSPECTIVE);

    Mtx model;
    guMtxIdentity(model);
    guMtxTransApply(model, model, 0.0f, 0.0f, -6.0f);
    GX_LoadPosMtxImm(model, GX_PNMTX0);

    while (SYS_MainLoop())
    {
        WPAD_ScanPads();

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;

        GX_SetViewport(0, 0, 640, 480, 0, 1);

        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(-1.0f, 1.0f, 0.0f);
        GX_Color3f32(0.2f, 0.8f, 1.0f);
        GX_Position3f32(1.0f, 1.0f, 0.0f);
        GX_Color3f32(0.2f, 0.8f, 1.0f);
        GX_Position3f32(1.0f, -1.0f, 0.0f);
        GX_Color3f32(0.2f, 0.8f, 1.0f);
        GX_Position3f32(-1.0f, -1.0f, 0.0f);
        GX_Color3f32(0.2f, 0.8f, 1.0f);
        GX_End();

        video.render();
    }

    return 0;
}
