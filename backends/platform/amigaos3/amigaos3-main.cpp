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

#include <stdio.h>

#include "common/scummsys.h"
#include "base/main.h"

#include "backends/platform/amigaos3/amigaos3-aga.h"


// Amiga includes.
#include <proto/icon.h>
#include <inline/icon.h>

#include <workbench/startup.h>


#define DEFAULT_AUDIO_THREAD_PRIORITY 0


const char *ID = "$VER: ScummVM 1.8.0\r\n";



static int wbClosed = 0;

struct CxBase* CxBase;
extern struct Library *CyberGfxBase;
struct GfxBase* GfxBase;
struct Library* IconBase;
struct IntuitionBase* IntuitionBase;
struct KeymapBase* KeymapBase;

static struct IORequest TimerDevice;

extern struct Device* TimerBase;
struct Device* TimerBase;

static void unload_libraries(void) {
    if (CxBase != NULL) {
        CloseLibrary((struct Library*) CxBase);
        CxBase = NULL;
    }

    if (CyberGfxBase != NULL) {
        CloseLibrary((struct Library*) CyberGfxBase);
        CyberGfxBase = NULL;
    }

    if (GfxBase != NULL) {
        CloseLibrary((struct Library*) GfxBase);
        GfxBase = NULL;
    }

    if (IconBase != NULL) {
        CloseLibrary((struct Library*) IconBase);
        IconBase = NULL;
    }

    if (IntuitionBase != NULL) {
        CloseLibrary((struct Library*) IntuitionBase);
        IntuitionBase = NULL;
    }

    if (KeymapBase != NULL) {
        CloseLibrary((struct Library*) KeymapBase);
        KeymapBase = NULL;
    }

    if (TimerBase != NULL) {
        CloseDevice(&TimerDevice);
        TimerBase = NULL;
    }
}

static void load_libraries(void) {
    atexit(unload_libraries);

    CxBase = (struct CxBase*) OpenLibrary("commodities.library", 0);
    if (CxBase == NULL) {
        fprintf(stderr, "Unable to load commodities.library!\n");
        exit(EXIT_FAILURE);
    }

    CyberGfxBase = (struct Library*) OpenLibrary("cybergraphics.library", 0);
    if (CyberGfxBase == NULL) {
        fprintf(stderr, "Unable to load cybergraphics.library!\n");
        exit(EXIT_FAILURE);
    }

    GfxBase = (struct GfxBase*) OpenLibrary("graphics.library", 0);
    if (GfxBase == NULL) {
        fprintf(stderr, "Unable to load graphics.library!\n");
        exit(EXIT_FAILURE);
    }

    IconBase = (struct Library*) OpenLibrary("icon.library", 0);
    if (IconBase == NULL) {
        fprintf(stderr, "Unable to load icon.library!\n");
        exit(EXIT_FAILURE);
    }

    IntuitionBase = (struct IntuitionBase*) OpenLibrary("intuition.library", 0);
    if (IntuitionBase == NULL) {
        fprintf(stderr, "Unable to load intuition.library!\n");
        exit(EXIT_FAILURE);
    }

    KeymapBase = (struct KeymapBase*) OpenLibrary("keymap.library", 0);
    if (KeymapBase == NULL) {
        fprintf(stderr, "Unable to load keymap.library!\n");
        exit(EXIT_FAILURE);
    }

    // Load timer.device so that GetSysTime is
    // available.
    OpenDevice("timer.device", 0, &TimerDevice, 37);
    if (TimerDevice.io_Device == NULL) {
        fprintf(stderr, "Unable to load timer.device!");
        exit(EXIT_FAILURE);
    }
    TimerBase = TimerDevice.io_Device;
}

int main(int argcWb, char *argvWb[]) {

    load_libraries();

    int audioThreadPriority = DEFAULT_AUDIO_THREAD_PRIORITY;
    struct WBStartup* wbStartup = NULL;
    int closeWb = 0;

    if (argcWb == 0) {
        wbStartup = (struct WBStartup*)argvWb;

    	// Process Tooltypes.
    	struct DiskObject *diskObject;
    	diskObject = GetDiskObject((char*)wbStartup->sm_ArgList[0].wa_Name);
    	if (diskObject != NULL) {
            char* toolType = (char*)FindToolType((char* const*)diskObject->do_ToolTypes, "AUDIO_THREAD_PRIORITY");
            if (toolType != NULL) {
        		sscanf(toolType, "%ld", &audioThreadPriority);
            }

            toolType = (char*)FindToolType((char* const*)diskObject->do_ToolTypes, "CLOSE_WB");
            if (toolType != NULL) {
        		closeWb = 1;
            }
        }
	}

	// Create our OSystem instance
	g_system = new OSystem_AmigaOS3();
    assert(g_system);

	// Pre initialize the backend
    ((OSystem_AmigaOS3 *)g_system)->init(audioThreadPriority);

    char *argv[] = {"ScummVM", NULL};
	int argc = sizeof(argv) / sizeof(char *) - 1;

    if (closeWb) {
	   wbClosed = CloseWorkBench();
    }

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);

	// Delete OSystem
	if (g_system) {
        delete (OSystem_AmigaOS3 *)g_system;
    }

	if (wbClosed) {
        OpenWorkBench();
    }

	return res;
}




