/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/scummsys.h"
#include "common/system.h"

#include "backends/platform/amigaos3/amigaos3-aga.h"

#include "backends/timer/default/default-timer.h"
#include "backends/events/amigaos3/amigaos3-events.h"

#include "common/textconsole.h"
#include "engines/engine.h"


// Amiga includes.
#include <proto/intuition.h>

//#include <clib/exec_protos.h>
#include <proto/exec.h>
#include <inline/exec.h>

//#include <clib/keymap_protos.h>
#include <proto/keymap.h>
#include <inline/keymap.h>

//#include "rawkeycodes.h"
#define RAWKEY_ESCAPE       0x45

#define RAWKEY_F1       0x50
#define RAWKEY_F2       0x51
#define RAWKEY_F3       0x52
#define RAWKEY_F4       0x53
#define RAWKEY_F5       0x54
#define RAWKEY_F6       0x55
#define RAWKEY_F7       0x56
#define RAWKEY_F8       0x57
#define RAWKEY_F9       0x58
#define RAWKEY_F10      0x59
#define RAWKEY_F11      0x4B
#define RAWKEY_F12      0x6F

#define RAWKEY_X        0x32
#define RAWKEY_Y        0x15
#define RAWKEY_Z        0x31

static DefaultTimerManager *_timerManager = NULL;

AmigaOS3EventSource::AmigaOS3EventSource() {
#ifndef NDEBUG
    debug(9, "AmigaOS3EventSource::AmigaOS3EventSource()");
#endif
    _timerManager = ((DefaultTimerManager *)g_system->getTimerManager());
}

AmigaOS3EventSource::~AmigaOS3EventSource() {
#ifndef NDEBUG
    debug(9, "AmigaOS3EventSource::~AmigaOS3EventSource()");
#endif
    _timerManager = NULL;
}



bool AmigaOS3EventSource::pollEvent(Common::Event &event) {
#ifndef NDEBUG
    debug(9, "AmigaOS3EventSource::pollEvent()");
#endif

	bool result = false;


    _timerManager->handler();


    struct Window *hardwareWindow = ((OSystem_AmigaOS3 *)g_system)->getHardwareWindow();


    if (hardwareWindow) {
        struct IntuiMessage *imsg;

        while (imsg = (struct IntuiMessage *)GetMsg(hardwareWindow->UserPort)) {

            ReplyMsg((struct Message *)imsg);

            switch (imsg->Class) {
                case IDCMP_MOUSEMOVE:
                    event.type = Common::EVENT_MOUSEMOVE;
                    event.mouse.x = imsg->MouseX;
                    event.mouse.y = imsg->MouseY;
                    ((OSystem_AmigaOS3 *)g_system)->setMouseCursorPosition(imsg->MouseX, imsg->MouseY);
                    result = true;
                    break;


                case IDCMP_MOUSEBUTTONS:
                    if (imsg->Code == SELECTDOWN) {
                        event.type = Common::EVENT_LBUTTONDOWN;
                    }

                    if (imsg->Code == SELECTUP) {
                        event.type = Common::EVENT_LBUTTONUP;
                    }

                    if (imsg->Code == MENUDOWN) {
                        event.type = Common::EVENT_RBUTTONDOWN;
                    }

                    if (imsg->Code == MENUUP) {
                        event.type = Common::EVENT_RBUTTONUP;
                    }

                    event.mouse.x = imsg->MouseX;
                    event.mouse.y = imsg->MouseY;
                    result = true;
                    break;


                case IDCMP_RAWKEY:
                    int code = imsg->Code;

                    int flags = 0;

                    if (imsg->Qualifier & (IEQUALIFIER_LALT | IEQUALIFIER_RALT)) {
                        flags |= Common::KBD_ALT;
                    }

                    if (imsg->Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)) {
                        flags |= Common::KBD_SHIFT;
                    }

                    if (imsg->Qualifier & IEQUALIFIER_CONTROL) {
                        flags |= Common::KBD_CTRL;
                    }

                    event.kbd.flags = flags;

                    event.type = (code & IECODE_UP_PREFIX) ? Common::EVENT_KEYUP : Common::EVENT_KEYDOWN;

                    code &= ~IECODE_UP_PREFIX;


                    switch (code) {
                        case RAWKEY_ESCAPE:
                            event.kbd.keycode = Common::KEYCODE_ESCAPE;
                            event.kbd.ascii = Common::ASCII_ESCAPE;
                            result = true;
                            break;

                        case RAWKEY_F1:
                            event.kbd.keycode = Common::KEYCODE_F1;
                            event.kbd.ascii = Common::ASCII_F1;
                            result = true;
                            break;

                        case RAWKEY_F2:
                            event.kbd.keycode = Common::KEYCODE_F2;
                            event.kbd.ascii = Common::ASCII_F2;
                            result = true;
                            break;

                        case RAWKEY_F3:
                            event.kbd.keycode = Common::KEYCODE_F3;
                            event.kbd.ascii = Common::ASCII_F3;
                            result = true;
                            break;

                        case RAWKEY_F4:
                            event.kbd.keycode = Common::KEYCODE_F4;
                            event.kbd.ascii = Common::ASCII_F4;
                            result = true;
                            break;

                        case RAWKEY_F5:
                            event.kbd.keycode = Common::KEYCODE_F5;
                            event.kbd.ascii = Common::ASCII_F5;
                            result = true;
                            break;

                        case RAWKEY_F6:
                            event.kbd.keycode = Common::KEYCODE_F6;
                            event.kbd.ascii = Common::ASCII_F6;
                            result = true;
                            break;

                        case RAWKEY_F7:
                            event.kbd.keycode = Common::KEYCODE_F7;
                            event.kbd.ascii = Common::ASCII_F7;
                            result = true;
                            break;

                        case RAWKEY_F8:
                            event.kbd.keycode = Common::KEYCODE_F8;
                            event.kbd.ascii = Common::ASCII_F8;
                            result = true;
                            break;

                        case RAWKEY_F9:
                            event.kbd.keycode = Common::KEYCODE_F9;
                            event.kbd.ascii = Common::ASCII_F9;
                            result = true;
                            break;

                        case RAWKEY_F10:
                            event.kbd.keycode = Common::KEYCODE_F10;
                            event.kbd.ascii = Common::ASCII_F10;
                            result = true;
                            break;

                        case RAWKEY_F11:
                            event.kbd.keycode = Common::KEYCODE_F11;
                            event.kbd.ascii = Common::ASCII_F11;
                            result = true;
                            break;

                        case RAWKEY_F12:
                            event.kbd.keycode = Common::KEYCODE_F12;
                            event.kbd.ascii = Common::ASCII_F12;
                            result = true;
                            break;

                        default:
                            if (flags == Common::KBD_CTRL && code == RAWKEY_Z) {
                                event.type = Common::EVENT_QUIT;
                            }

                            if (flags == Common::KBD_ALT && code == RAWKEY_X) {
                                event.type = Common::EVENT_QUIT;
                            }

                            InputEvent FakedIEvent;
                            memset(&FakedIEvent, 0, sizeof (InputEvent));
                            FakedIEvent.ie_Class = IECLASS_RAWKEY;
                            FakedIEvent.ie_Code = code;
                            FakedIEvent.ie_Qualifier = imsg->Qualifier;


                            char charbuf;

                            if (MapRawKey(&FakedIEvent, &charbuf, 1, NULL) == 1) {
                                event.kbd.ascii = charbuf;
                                event.kbd.keycode = (Common::KeyCode)charbuf;
                                result = true;
                            }
                            break;
                    }
            }
        }
    }

    return result;
}

