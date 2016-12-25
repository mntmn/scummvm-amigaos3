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



#include "backends/platform/amigaos3/amigaos3-aga.h"
#include "backends/fs/fs-factory.h"
#include "backends/fs/amigaos3/amigaos3-fs-node.h"
#include "graphics/scaler/aspect.h"
#include "common/scummsys.h"
#include "common/textconsole.h"
#include "common/debug.h"
#include "common/config-manager.h"


#include <proto/commodities.h>
#include <inline/commodities.h>

#include "amiga_c2p_aga.h"






static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Normal", GFX_NORMAL},
	{0, 0, 0}
};

static UWORD emptypointer[] = {
  0x0000, 0x0000,    /* reserved, must be NULL */
  0x0000, 0x0000,     /* 1 row of image data */
  0x0000, 0x0000    /* reserved, must be NULL */
};




#define AGA_VIDEO_DEPTH 8


static struct ScreenBuffer *_hardwareScreenBuffer[2] = {NULL, NULL};

void *c2p[2] = {NULL, NULL};

static BYTE _currentScreenBuffer = 0;





bool OSystem_AmigaOS3::hasFeature(OSystem::Feature f) {

    /*if (f == OSystem::kFeatureAspectRatioCorrection) {
        return true;
    }*/

    if (f == OSystem::kFeatureFullscreenMode) {
        return true;
    }

    return false;
}

void OSystem_AmigaOS3::setFeatureState(OSystem::Feature f, bool enable) {
#ifndef NDEBUG
    debug(4, "OSystem_AmigaOS3::setFeatureState()");
#endif

    /*switch (f) {
    	case OSystem::kFeatureAspectRatioCorrection:
            _videoMode.aspectRatioCorrectionRequested = enable;
    		break;
	}*/

}

bool OSystem_AmigaOS3::getFeatureState(OSystem::Feature f) {
#ifndef NDEBUG
    debug(4, "OSystem_AmigaOS3::getFeatureState()");

    assert (_transactionMode == kTransactionNone);
#endif


	switch (f) {
    	case OSystem::kFeatureFullscreenMode:
    		return true;

    	//case OSystem::kFeatureAspectRatioCorrection:
		//  return _videoMode.aspectRatioCorrectionRequested;

    	default:
    		return false;
	}
}

const OSystem::GraphicsMode *OSystem_AmigaOS3::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int OSystem_AmigaOS3::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

void OSystem_AmigaOS3::resetGraphicsScale() {
	setGraphicsMode(GFX_NORMAL);
}

void OSystem_AmigaOS3::beginGFXTransaction() {
#ifndef NDEBUG
    debug(4, "OSystem_AmigaOS3::beginGFXTransaction()");

	assert(_transactionMode == kTransactionNone);
#endif


	_transactionMode = kTransactionActive;

	_transactionDetails.sizeChanged = false;

    // Store the current mode in the old mode.
	_oldVideoMode = _videoMode;
}

OSystem::TransactionError OSystem_AmigaOS3::endGFXTransaction() {
#ifndef NDEBUG
    debug(4, "OSystem_AmigaOS3::endGFXTransaction()");

    assert(_transactionMode != kTransactionNone);
#endif


	int errors = OSystem::kTransactionSuccess;

	if (_transactionMode == kTransactionRollback) {
    	errors |= OSystem::kTransactionSizeChangeFailed;

        // Revert to the last (working) mode.
		_videoMode.screenWidth = _oldVideoMode.screenWidth;
		_videoMode.screenHeight = _oldVideoMode.screenHeight;

		// Stop an endless loop if loadGFXMode() also fails with the old mode.
		_oldVideoMode.setup = false;
    }


    if (_transactionDetails.sizeChanged) {
        unloadGFXMode();
        if(!loadGFXMode()) {
           	if (_oldVideoMode.setup) {
                _transactionMode = kTransactionRollback;
				errors |= endGFXTransaction();
			} else {
                error("Could not switch to video mode (%d x %d)", _videoMode.screenWidth, _videoMode.screenHeight);
            }
        }

        // Success.
        SetPointer(_hardwareWindow, emptypointer, 1, 16, 0, 0);


        // Set current cursor position.
        _mouseCursor.x = _hardwareWindow->MouseX;
        _mouseCursor.y = _hardwareWindow->MouseY;


        if (!_overlayPalette) {
            _overlayPalette = (byte *)calloc(3 * 256, sizeof(byte));
            loadOverlayPalette();
        }

        setPalette((byte*)_overlayPalette, 0, 256);




        if (!_overlayColorMap) {
            _overlayColorMap = (byte*)calloc(65536, sizeof(byte));
            loadOverlayColorMap();
        }

        _videoMode.setup = true;

        _screenChangeCount++;
    }

	_transactionMode = kTransactionNone;


    return (OSystem::TransactionError)errors;
}

bool OSystem_AmigaOS3::setGraphicsMode(int mode) {
#ifndef NDEBUG
    debug(4, "OSystem_AmigaOS3::setGraphicsMode(%d)", mode);
#endif

	assert(_transactionMode == kTransactionActive);

	if (_oldVideoMode.setup && _oldVideoMode.mode == mode) {
		return true;
    }

	if (mode != GFX_NORMAL) {
        warning("Unsupported GFX mode %d", mode);
        return false;
    }

	_videoMode.mode = mode;

	return true;
}

int OSystem_AmigaOS3::getGraphicsMode() const {
	assert (_transactionMode == kTransactionNone);

	return _videoMode.mode;
}

void OSystem_AmigaOS3::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
#ifndef NDEBUG
    debug(4, "OSystem_AmigaOS3::initSize(%d, %d)", w, h);

	assert(_transactionMode == kTransactionActive);
#endif

	// Avoid redundant res changes
    /*if (_videoMode.aspectRatioCorrectionRequested) {
        if (w == _videoMode.screenWidth && real2Aspect(h) == _videoMode.scaledHeight) {
    		return;
        }
    } else {*/
         if (w == _videoMode.screenWidth && h == _videoMode.screenHeight) {
    		return;
        }
    //}

	_videoMode.screenWidth = w;
	_videoMode.screenHeight = h;

	_transactionDetails.sizeChanged = true;
}

bool OSystem_AmigaOS3::loadGFXMode() {
#ifndef NDEBUG
    debug(4, "OSystem_AmigaOS3::loadGFXMode()");

    debug(5, "New video mode requested - width = %d, height = %d", _videoMode.screenWidth, _videoMode.screenHeight);
#endif

    // Reset.
    //_videoMode.aspectRatioCorrectionInUse = false;

    // Only aspect scale 200 pixel heights.
	/*if (_videoMode.aspectRatioCorrectionRequested && _videoMode.screenHeight == 200) {
		debug(5, "Aspect ratio correction requested and allowed");
		_videoMode.aspectRatioCorrectionInUse = true;

		// Adjust screen height.
        //_videoMode.screenHeight = real2Aspect(_videoMode.screenHeight);
        _videoMode.scaledHeight = real2Aspect(_videoMode.screenHeight);
        debug(5, "Aspect corrected video mode - width = %d, height = %d", _videoMode.screenWidth, _videoMode.screenHeight);
    }*/



    // Default overlay size to the same as the (game) screen.
    _videoMode.overlayScreenHeight = _videoMode.screenHeight;

    // Increase for a bigger overlay.
    if (_videoMode.overlayScreenHeight < 256) {
        _videoMode.overlayScreenHeight = 256;
    }



    // Create the hardware screen.
    _hardwareScreen = createHardwareScreen();
    if (!_hardwareScreen) {
		return false;
    }

    // Setup double buffering.
    _hardwareScreenBuffer[0] = AllocScreenBuffer (_hardwareScreen, NULL, SB_SCREEN_BITMAP);
    if (!_hardwareScreenBuffer[0]) {
		return false;
    }

    _hardwareScreenBuffer[1] = AllocScreenBuffer (_hardwareScreen, NULL, 0);
    if (!_hardwareScreenBuffer[1]) {
		return false;
    }


	_currentScreenBuffer = 1;

    // Setup C2P.
    c2p[0] = c2p8_reloc_stub(_hardwareScreenBuffer[0]->sb_BitMap);
    c2p[1] = c2p8_reloc_stub(_hardwareScreenBuffer[1]->sb_BitMap);

    // Create the hardware window.
    _hardwareWindow = createHardwareWindow();
    if (!_hardwareWindow) {
		return false;
    }

    // Create the surface that contains the 8 bit game data
    _screen.create(_videoMode.screenWidth, _videoMode.screenHeight, Graphics::PixelFormat::createFormatCLUT8());

    // Create the screen used by the scaler/shaker.
    _tmpscreen.create(_videoMode.screenWidth, _videoMode.screenHeight, Graphics::PixelFormat::createFormatCLUT8());



    // Create the 8bit overlay surface
    _overlayscreen8.create(_videoMode.screenWidth, _videoMode.overlayScreenHeight, Graphics::PixelFormat::createFormatCLUT8());

	// Create the 16bit overlay surface
    _overlayscreen16.create(_videoMode.screenWidth, _videoMode.overlayScreenHeight, _overlayFormat);


    return true;
}

struct Screen* OSystem_AmigaOS3::createHardwareScreen() {

    // Create the hardware screen.
    struct Screen* screen = NULL;
    ULONG modeId = INVALID_ID;




    modeId = BestModeID(BIDTAG_NominalWidth, _videoMode.screenWidth,
                        BIDTAG_NominalHeight, _videoMode.overlayScreenHeight,
            	        BIDTAG_DesiredWidth, _videoMode.screenWidth,
            	        BIDTAG_DesiredHeight, _videoMode.overlayScreenHeight,
            	        BIDTAG_Depth, AGA_VIDEO_DEPTH,
            	        BIDTAG_MonitorID, PAL_MONITOR_ID,
            	        TAG_END);

    if (modeId != INVALID_ID) {
    	screen = OpenScreenTags(NULL,
                         SA_Depth, AGA_VIDEO_DEPTH,
                         SA_DisplayID, modeId,
                         SA_Width, _videoMode.screenWidth,
    					 SA_Height, _videoMode.overlayScreenHeight,
						 SA_Type, CUSTOMSCREEN,
                         SA_Quiet, TRUE,
    					 SA_ShowTitle, FALSE,
    					 SA_Draggable, FALSE,
                         SA_Exclusive, TRUE,
    					 SA_AutoScroll, FALSE,
						 TAG_END);
    }

    return screen;
}

struct Window* OSystem_AmigaOS3::createHardwareWindow() {

    return OpenWindowTags(NULL,
                  	    WA_Left, 0,
            			WA_Top, 0,
            			WA_Width, _videoMode.screenWidth,
            			WA_Height, _videoMode.overlayScreenHeight,
    					SA_AutoScroll, FALSE,
            			WA_CustomScreen, (ULONG)_hardwareScreen,
            			WA_Backdrop, TRUE,
            			WA_Borderless, TRUE,
            			WA_DragBar, FALSE,
            			WA_Activate, TRUE,
            			WA_SimpleRefresh, TRUE,
            			WA_NoCareRefresh, TRUE,
            			WA_ReportMouse, TRUE,
            			WA_RMBTrap, TRUE,
                  	    WA_IDCMP, IDCMP_RAWKEY|IDCMP_MOUSEMOVE|IDCMP_MOUSEBUTTONS,
                  	    TAG_END);
}


void OSystem_AmigaOS3::unloadGFXMode() {

	_screen.free();
	_tmpscreen.free();

	_overlayscreen8.free();
	_overlayscreen16.free();



    if (_hardwareWindow) {
        ClearPointer(_hardwareWindow);
        CloseWindow(_hardwareWindow);
        _hardwareWindow = NULL;
    }

    if (_hardwareScreenBuffer[0] != NULL) {
        ChangeScreenBuffer (_hardwareScreen, _hardwareScreenBuffer[0]);
        WaitTOF();
        WaitTOF();
        FreeScreenBuffer (_hardwareScreen, _hardwareScreenBuffer[0]);
        _hardwareScreenBuffer[0] = NULL;
    }

    if (_hardwareScreenBuffer[1] != NULL) {
        FreeScreenBuffer (_hardwareScreen, _hardwareScreenBuffer[1]);
        _hardwareScreenBuffer[1] = NULL;
    }

    if (_hardwareScreen) {
        CloseScreen(_hardwareScreen);
        _hardwareScreen = NULL;
    }

    if (c2p[0]) {
        c2p8_deinit_stub(c2p[0]);
        c2p[0] = NULL;
    }

    if (c2p[1]) {
        c2p8_deinit_stub(c2p[1]);
        c2p[1] = NULL;
    }
}


void OSystem_AmigaOS3::setPalette(const byte *colors, uint start, uint num) {
#ifndef NDEBUG
	debug(4, "OSystem_AmigaOS3:setPalette()");

    debug(5, "setPalette() - start = %d", start);
	debug(5, "setPalette() - num = %d", num);

    assert(colors);
#endif


	byte *dst = (byte*)(_currentPalette + (3 * start));
    CopyMem((byte*)colors, dst, (num * 3));

	if (start < _paletteDirtyStart) {
		_paletteDirtyStart = start;
    }

    if (start + num > _paletteDirtyEnd) {
		_paletteDirtyEnd = start + num;
    }
}

void OSystem_AmigaOS3::grabPalette(byte *colors, uint start, uint num) {
#ifndef NDEBUG
	assert(colors);
#endif

    CopyMem(_currentPalette + (3 * start), colors, 3 * num);
}


void OSystem_AmigaOS3::updatePalette() {
#ifndef NDEBUG
    debug(4, "updatePalette()");

    debug(5, "updatePalette() - _paletteDirtyStart = %d", _paletteDirtyStart);
	debug(5, "updatePalette() - _paletteDirtyEnd = %d", _paletteDirtyEnd);
#endif

    uint j = 1;
    byte *color = (byte*)(_currentPalette + 3 * _paletteDirtyStart);

    for(uint i = _paletteDirtyStart; i < _paletteDirtyEnd; i++) {

        _agaPalette[j] = color[0] << 24;
    	_agaPalette[j+1] = color[1] << 24;
    	_agaPalette[j+2] = color[2] << 24;

    	j += 3;
    	color += 3;
    }

	uint numberOfEntries = (_paletteDirtyEnd - _paletteDirtyStart);

    _agaPalette[0] = (numberOfEntries << 16) + _paletteDirtyStart;

    // Terminator: NEEDED
    _agaPalette[((numberOfEntries * 3) + 1)] = 0x00000000;

	LoadRGB32(&_hardwareScreen->ViewPort, _agaPalette);

    // Reset.
    _paletteDirtyStart = 256;
    _paletteDirtyEnd = 0;
}




void OSystem_AmigaOS3::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
#ifndef NDEBUG
    debug(4, "copyRectToScreen()");
    debug(5, "copyRectToScreen() - pitch = %d", pitch);
    debug(5, "copyRectToScreen() - x = %d", x);
    debug(5, "copyRectToScreen() - y = %d", y);
    debug(5, "copyRectToScreen() - w = %d", w);
    debug(5, "copyRectToScreen() - h = %d", h);

    assert(_transactionMode == kTransactionNone);
	assert(buf);


	assert(x >= 0 && x < _videoMode.screenWidth);
	assert(y >= 0 && y < _videoMode.screenHeight);
	assert(h > 0 && y + h <= _videoMode.screenHeight);
	assert(w > 0 && x + w <= _videoMode.screenWidth);
#endif


	byte *dst = (byte*)_screen.getBasePtr(x, y);

    if (_videoMode.screenWidth == pitch && pitch == w) {
        CopyMemQuick((byte*)buf, dst, w * h);
	} else {
		const byte *src = (const byte *)buf;
		do {
			CopyMem((void *)src, dst, w);
			src += pitch;
			dst += _videoMode.screenWidth;//_screen.pitch;
		} while (--h);
	}
}





void OSystem_AmigaOS3::fillScreen(uint32 col) {
    if (_screen.pixels) {
        memset(_screen.pixels, (int)col, (_videoMode.screenWidth * _videoMode.screenHeight));
    }
}




void OSystem_AmigaOS3::updateScreen() {
#ifndef NDEBUG
    debug(9, "OSystem_AmigaOS3::updateScreen()");
#endif

    static UBYTE* src;


    if (_mouseCursor.visible) {
        drawMouse();
    }

    if (_overlayVisible) {
        src = (UBYTE*)_overlayscreen8.pixels;

        c2p8_stub(c2p[_currentScreenBuffer], _hardwareScreenBuffer[_currentScreenBuffer]->sb_BitMap, src, _videoMode.screenWidth * _videoMode.overlayScreenHeight);
    } else {
        if (_currentShakePos != _newShakePos) {
            // Set the 'dirty area' to black.
            memset(_tmpscreen.getBasePtr(0, (_videoMode.screenHeight - _newShakePos)), 0, (_videoMode.screenWidth * _newShakePos));

            src = (UBYTE*)_screen.getBasePtr(0, _newShakePos);
            byte *dst = (byte*)_tmpscreen.getBasePtr(0, 0);

            CopyMemQuick(src, dst, (_videoMode.screenWidth * (_videoMode.screenHeight - _newShakePos)));

            // Reset.
	        _currentShakePos = _newShakePos;

	        src = (UBYTE*)_tmpscreen.pixels;
        } else {
            src = (UBYTE*)_screen.pixels;
        }

        c2p8_stub(c2p[_currentScreenBuffer], _hardwareScreenBuffer[_currentScreenBuffer]->sb_BitMap, src, _videoMode.screenWidth * _videoMode.screenHeight);
    }



    // Check whether the palette was changed.
	if (_paletteDirtyEnd != 0) {
        updatePalette();
	}


    if (_mouseCursor.visible) {
        undrawMouse();
    }

    if (ChangeScreenBuffer(_hardwareScreen, _hardwareScreenBuffer[_currentScreenBuffer])) {
        // Flip.
	   _currentScreenBuffer = _currentScreenBuffer ^ 1;
    }
}



void OSystem_AmigaOS3::setShakePos(int shakeOffset) {
#ifndef NDEBUG
	assert (_transactionMode == kTransactionNone);
#endif

	_newShakePos = shakeOffset;
}













#pragma mark -
#pragma mark --- Overlays ---
#pragma mark -

void OSystem_AmigaOS3::loadOverlayPalette() {

	// Load overlay palette file.
   	FILE *paletteFile;

    paletteFile = fopen("overlay.pal", "r");
    if (paletteFile == NULL) {
        error("Could not load the palette file");
    } else {
        // Skip forward 3 rows.
        char temp[100];
        fgets(temp, 100, paletteFile);
        fgets(temp, 100, paletteFile);
        fgets(temp, 100, paletteFile);

        // Read the palette data.
        int red, green, blue;

        byte *color = _overlayPalette;

        while (fscanf(paletteFile, "%d %d %d", &red, &green, &blue) != EOF) {
           color[0] = red;
    	   color[1] = green;
    	   color[2] = blue;

    	   color += 3;
        }

        fclose(paletteFile);
    }
}

void OSystem_AmigaOS3::loadOverlayColorMap() {
#ifndef NDEBUG
    debug(4, "generateOverlayColorMap()");
#endif

    int color8;

    // Load overlay map file.
   	FILE *mapFile;

    mapFile = fopen("overaly.map", "r");
    if (mapFile == NULL) {
        error("Could not load the overaly map file");
    }


    int i = 0;

    while (fscanf(mapFile, "%d", &color8) != EOF) {
        _overlayColorMap[i] = color8;
        i++;
    }

    fclose(mapFile);
}

void OSystem_AmigaOS3::showOverlay() {
#ifndef NDEBUG
	assert (_transactionMode == kTransactionNone);
#endif

	if (_overlayVisible) {
		return;
    }

	_overlayVisible = true;

    // Make a backup of the current game palette.
    memcpy(_gamePalette, _currentPalette, PALETTE_SIZE);

    // Set the overlay palette.
    setPalette((byte*)_overlayPalette, 0, 256);
}

void OSystem_AmigaOS3::hideOverlay() {
#ifndef NDEBUG
	assert (_transactionMode == kTransactionNone);
#endif

	if (!_overlayVisible) {
		return;
    }


    clearOverlay();

    UBYTE* src = (UBYTE*)_overlayscreen8.pixels;
    c2p8_stub(c2p[_currentScreenBuffer], _hardwareScreenBuffer[_currentScreenBuffer]->sb_BitMap, src, _videoMode.screenWidth * _videoMode.overlayScreenHeight);

    if (ChangeScreenBuffer(_hardwareScreen, _hardwareScreenBuffer[_currentScreenBuffer])) {
        // Flip.
	   _currentScreenBuffer = _currentScreenBuffer ^ 1;

	   c2p8_stub(c2p[_currentScreenBuffer], _hardwareScreenBuffer[_currentScreenBuffer]->sb_BitMap, src, _videoMode.screenWidth * _videoMode.overlayScreenHeight);
    }


	_overlayVisible = false;

    // Reset the game palette.
    setPalette((byte*)_gamePalette, 0, 256);
}

void OSystem_AmigaOS3::clearOverlay() {

	if (!_overlayVisible) {
		return;
    }

    // Set the background to black.
    byte *src = (byte*)_overlayscreen8.pixels;
    memset(src, 0, (_videoMode.screenWidth * _videoMode.overlayScreenHeight));
}

void OSystem_AmigaOS3::grabOverlay(void *buf, int pitch) {
#ifndef NDEBUG
	assert (_transactionMode == kTransactionNone);
#endif

    // Grab the overlay.
    memcpy(buf, _overlayscreen16.pixels, (_videoMode.screenWidth * _videoMode.overlayScreenHeight) * _overlayscreen16.format.bytesPerPixel);
}

void OSystem_AmigaOS3::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
#ifndef NDEBUG
    debug(4, "copyRectToOverlay()");

   	assert (_transactionMode == kTransactionNone);
#endif

	// Clip the coordinates
    if (x < 0) {
		return;
	}

	if (y < 0) {
		return;
	}

	if (w > _videoMode.screenWidth - x) {
		w = _videoMode.screenWidth - x;
	}

	if (h > _videoMode.overlayScreenHeight - y) {
		h = _videoMode.overlayScreenHeight - y;
	}

	if (w <= 0 || h <= 0) {
		return;
    }


    const OverlayColor *src = (const OverlayColor *)buf;
    byte *dst = (byte*)_overlayscreen8.getBasePtr(x, y);

    OverlayColor color16;
    byte color8;

    for(uint r = 0; r < h; r++) {
        for(uint c = 0; c < w; c++) {
            color16 = *src;

            color8 = _overlayColorMap[color16];

            *dst = color8;

            // Add a column.
            src++;
            dst++;
        }

        // add a row.
        dst += (_videoMode.screenWidth - w);
        src += (_videoMode.screenWidth - w);
    }
}





#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

bool OSystem_AmigaOS3::showMouse(bool visible) {
	if (_mouseCursor.visible == visible) {
		return visible;
    }

	bool last = _mouseCursor.visible;
	 _mouseCursor.visible = visible;

	return last;
}

void OSystem_AmigaOS3::warpMouse(int x, int y) {

	struct InputEvent ie;
	struct IEPointerPixel pp;

	ie.ie_NextEvent = NULL;
    ie.ie_Class = IECLASS_NEWPOINTERPOS;
    ie.ie_SubClass = IESUBCLASS_PIXEL;
    ie.ie_Code = 0;
    ie.ie_Qualifier = 0;
    ie.ie_EventAddress = &pp;

    pp.iepp_Screen = _hardwareScreen;
    pp.iepp_Position.X = x;
    pp.iepp_Position.Y = y;

	AddIEvents(&ie);
}

void OSystem_AmigaOS3::setMouseCursor(const void *buf, uint w, uint h,
 int hotspot_x, int hotspot_y, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
#ifndef NDEBUG
    debug(4, "OSystem_AmigaOS3::setMouseCursor(w = %d, h = %d)", w, h);

    assert(buf);
#endif

    // Sanity check.
	if (w == 0 || h == 0) {
		return;
    }

    // Check to see if we need to recreate the surfaces.
    if (w != _mouseCursor.w || h != _mouseCursor.h) {
        _mouseCursor.surface.create(w, h, Graphics::PixelFormat::createFormatCLUT8());
        _mouseCursorMask.surface.create(w, h, Graphics::PixelFormat::createFormatCLUT8());
    }

    _mouseCursor.w = w;
    _mouseCursor.h = h;
    _mouseCursor.hotX = hotspot_x;
    _mouseCursor.hotY = hotspot_y;
    _mouseCursor.keyColor = keycolor;

    CopyMem((void*)buf, _mouseCursor.surface.pixels, w * h);
}

void OSystem_AmigaOS3::setMouseCursorPosition(uint x, uint y) {
    _mouseCursor.x = x;
    _mouseCursor.y = y;
}

void OSystem_AmigaOS3::drawMouse() {
#ifndef NDEBUG
    debug(4, "OSystem_AmigaOS3::drawMouse()");
#endif

    uint w = _mouseCursor.w;
    uint h = _mouseCursor.h;

    int x = (_mouseCursor.x - _mouseCursor.hotX);
    int y = (_mouseCursor.y - _mouseCursor.hotY);


    byte *mousePixels = (byte*)_mouseCursor.surface.pixels;


    // Clip the coordinates
	if (x < 0) {
		w += x;
		mousePixels -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
        mousePixels -= (y * _mouseCursor.surface.pitch);
		y = 0;
	}


	if (w > _videoMode.screenWidth - x) {
		w = _videoMode.screenWidth - x;
	}

	if (h > _videoMode.screenHeight - y) {
		h = _videoMode.screenHeight - y;
	}


	if (w <= 0 || h <= 0) {
		// Nothing to do.
        return;
    }


    // Setup the cursor mask.
    _mouseCursorMask.x = x;
    _mouseCursorMask.y = y;
    _mouseCursorMask.w = w;
    _mouseCursorMask.h = h;

    byte *maskPixels = (byte*)_mouseCursorMask.surface.pixels;


    // Set the starting point of the screen we will be drawing to.
    byte *screenPixels = NULL;

    if(_overlayVisible) {
        screenPixels = (byte*)_overlayscreen8.getBasePtr(x, y);
    } else {
        screenPixels = (byte*)_screen.getBasePtr(x, y);
    }


	// Draw it.
    byte color;

	do {
		// Save a copy of this row before it's overwritten.
        CopyMem(screenPixels, maskPixels, w);

         for(uint c = 0; c < w; c++) {
            color = *mousePixels;

            if (color != _mouseCursor.keyColor) {
                // Set the color.
                *screenPixels = color;
            }

            // Add a column.
            mousePixels++;
            screenPixels++;
        }

        // add a row.
        maskPixels += w;
        mousePixels += (_mouseCursor.surface.pitch - w);
        screenPixels += (_videoMode.screenWidth - w);
	} while (--h);
}

void OSystem_AmigaOS3::undrawMouse() {
#ifndef NDEBUG
    debug(4, "OSystem_AmigaOS3::undrawMouse()");
#endif


    byte *dst = NULL;

    if(_overlayVisible) {
        dst = (byte*)_overlayscreen8.getBasePtr(_mouseCursorMask.x, _mouseCursorMask.y);
    } else {
        dst = (byte*)_screen.getBasePtr(_mouseCursorMask.x, _mouseCursorMask.y);
    }

    byte *src = (byte*)_mouseCursorMask.surface.pixels;

    for(uint i = 0; i < _mouseCursorMask.h; i++) {
        CopyMem(src, dst, _mouseCursorMask.w);
        dst += _videoMode.screenWidth;
        src += _mouseCursorMask.w;
    }
}

/*UBYTE *OSystem_AmigaOS3::scaleScreen() {

    byte *src;
    byte *dst = (byte*)_tmpscreen.getBasePtr(0, _videoMode.scaledHeight - 1);

    bool shaking = false;

	// If the shake position changed, fill the dirty area with blackness
    if (_currentShakePos != _newShakePos) {

        // Set the 'dirty area' to black.
        memset(_tmpscreen.getBasePtr(0, (_videoMode.scaledHeight - _newShakePos)), 0, (_videoMode.screenWidth * _newShakePos));

        for (uint y = (_videoMode.scaledHeight - 1); y > _newShakePos; y--) {
            src = (byte *)_screen.pixels + (aspect2Real(y + _newShakePos) * _videoMode.screenWidth);

            CopyMemQuick(src, dst, _videoMode.screenWidth);

            dst -= _videoMode.screenWidth;
        }

        // Reset.
    	_currentShakePos = _newShakePos;
    } else {
        for (uint y = (_videoMode.scaledHeight - 1); y > 0; y--) {
            src = (byte *)_screen.pixels + (aspect2Real(y) * _videoMode.screenWidth);

            CopyMemQuick(src, dst, _videoMode.screenWidth);

            dst -= _videoMode.screenWidth;
        }
    }

    return (UBYTE*)_tmpscreen.pixels;
}*/
