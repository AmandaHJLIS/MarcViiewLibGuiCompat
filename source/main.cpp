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
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetNumChans(1);
    GX_SetNumTexGens(0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);

    Mtx44 projection;
    guPerspective(projection, 45.0f, 640.0f / 480.0f, 0.1f, 300.0f);
    GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

    Mtx model;
    guMtxIdentity(model);
    guMtxTransApply(model, model, 0.0f, 0.0f, -6.0f);
    GX_LoadPosMtxImm(model, GX_PNMTX0);

    while (SYS_MainLoop())
    {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)
            break;

        video.getImageRenderer()->drawRectangle(
            -1.0f, -1.0f, 2.0f, 2.0f, {40, 160, 220, 255});

        video.render();
    }

    return 0;
}
