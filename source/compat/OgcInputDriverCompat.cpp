/****************************************************************************
 * MarcViiewLibGuiCompat - legacy Wii input compatibility
 *
 * Minimal OGC input backend for the legacy libogc environment.
 * This deliberately starts with the Wii Remote path only. It exercises
 * upstream libgui's InputDriver -> InputController -> GuiTrigger/GuiButton
 * pipeline without pulling in the newer WiiDRC or GameCube-specific pieces.
 ****************************************************************************/

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogcsys.h>

#include "drivers/ogc/OgcInputDriver.h"
#include "../drivers/InputController.h"
#include "../drivers/Platform.h"

static uint32_t MapWiimoteButtons(uint32_t buttons)
{
    uint32_t mask = INPUT_BTN_NONE;

    if (buttons & WPAD_BUTTON_A)     mask |= INPUT_BTN_A;
    if (buttons & WPAD_BUTTON_B)     mask |= INPUT_BTN_B;
    if (buttons & WPAD_BUTTON_1)     mask |= INPUT_BTN_1;
    if (buttons & WPAD_BUTTON_2)     mask |= INPUT_BTN_2;
    if (buttons & WPAD_BUTTON_UP)    mask |= INPUT_BTN_UP;
    if (buttons & WPAD_BUTTON_DOWN)  mask |= INPUT_BTN_DOWN;
    if (buttons & WPAD_BUTTON_LEFT)  mask |= INPUT_BTN_LEFT;
    if (buttons & WPAD_BUTTON_RIGHT) mask |= INPUT_BTN_RIGHT;
    if (buttons & WPAD_BUTTON_PLUS)  mask |= INPUT_BTN_PLUS;
    if (buttons & WPAD_BUTTON_MINUS) mask |= INPUT_BTN_MINUS;
    if (buttons & WPAD_BUTTON_HOME)  mask |= INPUT_BTN_HOME;

    return mask;
}

OgcInputDriver::OgcInputDriver()
{
    for (int i = 0; i < 4; ++i) {
        rumbleRequest[i] = false;
        menuRumbleFrames[i] = 0;
        menuRumbleGapFrames[i] = 0;
    }
}

OgcInputDriver::~OgcInputDriver()
{
    shutdown();
}

void OgcInputDriver::init()
{
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(
        WPAD_CHAN_ALL,
        platform->getVideo()->getScreenWidth(),
        platform->getVideo()->getScreenHeight());

    InitUserInputControllers();
}

void OgcInputDriver::shutdown()
{
    for (int i = 0; i < 4; ++i) {
        WPAD_Rumble(i, 0);
        rumbleRequest[i] = false;
        menuRumbleFrames[i] = 0;
        menuRumbleGapFrames[i] = 0;
    }
}

void OgcInputDriver::setRumble(int channel, bool rumble)
{
    if (channel >= 0 && channel < 4)
        rumbleRequest[channel] = rumble;
}

void OgcInputDriver::update()
{
    WPAD_ScanPads();

    for (int i = 0; i < 4; ++i) {
        InputPadData data;

        uint32_t expType = WPAD_EXP_NONE;

        if (WPAD_Probe(i, &expType) == WPAD_ERR_NONE) {
            WPADData* wpad = WPAD_Data(i);

            data.hw_connected[INPUT_HW_WIIMOTE] = true;
            data.hw_buttons_d[INPUT_HW_WIIMOTE] =
                MapWiimoteButtons(wpad->btns_d);
            data.hw_buttons_h[INPUT_HW_WIIMOTE] =
                MapWiimoteButtons(wpad->btns_h);
            data.hw_buttons_r[INPUT_HW_WIIMOTE] =
                MapWiimoteButtons(wpad->btns_u);

            data.battery_level = wpad->battery_level;

            data.hw_gforceX[INPUT_HW_WIIMOTE] = wpad->gforce.x;
            data.hw_gforceY[INPUT_HW_WIIMOTE] = wpad->gforce.y;
            data.hw_gforceZ[INPUT_HW_WIIMOTE] = wpad->gforce.z;
            data.hw_pitch[INPUT_HW_WIIMOTE] = wpad->orient.pitch;
            data.hw_roll[INPUT_HW_WIIMOTE] = wpad->orient.roll;
            data.hw_yaw[INPUT_HW_WIIMOTE] = wpad->orient.yaw;

            if (wpad->ir.valid) {
                data.validPointer = true;
                data.cursor_x = wpad->ir.x;
                data.cursor_y = wpad->ir.y;
                data.cursor_angle = wpad->ir.angle;
            }
        }

        for (uint32_t hw = 0; hw < INPUT_HW_MAX; ++hw) {
            if (!data.hw_connected[hw])
                continue;

            data.buttons_d |= data.hw_buttons_d[hw];
            data.buttons_h |= data.hw_buttons_h[hw];
            data.buttons_r |= data.hw_buttons_r[hw];
        }

        controller[i]->setSideways(
            getWiimoteOrientation() == WIIMOTE_ORIENTATION_HORIZONTAL);
        controller[i]->update(data, platform->getVideo()->getDeltaTime());
    }
}
