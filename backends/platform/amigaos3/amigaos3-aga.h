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

#ifndef PLATFORM_AMIGAOS3_H
#define PLATFORM_AMIGAOS3_H

#include <time.h>

#include "audio/mixer_intern.h"
#include "backends/log/log.h"
#include "backends/events/amigaos3/amigaos3-events.h"
#include "backends/mixer/amigaos3/amigaos3-mixer.h"

#include "common/scummsys.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "graphics/palette.h"

// Amiga includes.
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>

#define PALETTE_SIZE 768

enum {
  GFX_NORMAL = 0
};

class OSystem_AmigaOS3 : public OSystem, PaletteManager {
public:
  OSystem_AmigaOS3();

  ~OSystem_AmigaOS3();

  /**
   * Pre-initialize backend. It should be called after
   * instantiating the backend. Early needed managers are
   * created here.
   */
  void init(int audioThreadPriority);


  // Override functions from ModularBackend and OSystem
  void initBackend();

  void quit();

  // Logging
  void logMessage(LogMessageType::Type type, const char *message);


  Common::SeekableReadStream *createConfigReadStream();
  Common::WriteStream *createConfigWriteStream();

  virtual uint32 getMillis(bool skipRecord = false);

  void delayMillis(uint msecs);

  void getTimeAndDate(TimeDate &td) const;




  bool hasFeature(OSystem::Feature f);
  void setFeatureState(OSystem::Feature f, bool enable);
  bool getFeatureState(OSystem::Feature f);


  const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
  int getDefaultGraphicsMode() const;
  bool setGraphicsMode(int mode);
  void resetGraphicsScale();
  int getGraphicsMode() const;


  void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL);

  int getScreenChangeID() const { return _screenChangeCount; }

  void beginGFXTransaction();
  OSystem::TransactionError endGFXTransaction();

  inline int16 getHeight() { return _videoMode.screenHeight; }
  inline int16 getWidth() { return _videoMode.screenWidth; }

  inline PaletteManager *getPaletteManager() { return this; }

  void setPalette(const byte *colors, uint start, uint num);
  void grabPalette(byte *colors, uint start, uint num);

  void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h);
  inline Graphics::Surface *lockScreen() { return &_screen; }
  void unlockScreen() { _screenDirty = true; }
  void fillScreen(uint32 col);
  void updateScreen();
  void setShakePos(int shakeOffset);

  void setFocusRectangle(const Common::Rect& rect) {}
  void clearFocusRectangle() {}

  void showOverlay();
  void hideOverlay();
  inline Graphics::PixelFormat getOverlayFormat() const { return _overlayFormat; }
  void clearOverlay();
  void grabOverlay(void *buf, int pitch);
  void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h);
  inline int16 getOverlayHeight() { return _videoMode.overlayHeight; }
  inline int16 getOverlayWidth() { return _videoMode.overlayWidth; }

  bool showMouse(bool visible);
  void warpMouse(int x, int y);

  void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor,
                      bool dontScale = false, const Graphics::PixelFormat *format = NULL);

  void setCursorPalette(const byte *colors, uint start, uint num) {}
  void disableCursorPalette(bool disable) {}
  void setMouseCursorPosition(uint x, uint y);

  //inline struct Window* getHardwareWindow() { return _hardwareWindow; }
  struct Window* getHardwareWindow();

  inline Audio::Mixer *getMixer() { return _mixerManager->getMixer(); }

  void displayMessageOnOSD(const char *msg) {}



  MutexRef createMutex();
  inline void lockMutex(OSystem::MutexRef mutex) { ObtainSemaphore((SignalSemaphore *) mutex); }
  inline void unlockMutex(OSystem::MutexRef mutex) { ReleaseSemaphore((SignalSemaphore *)mutex); }
  inline void deleteMutex(OSystem::MutexRef mutex) { FreeVec(mutex); }


protected:
  bool _inited;


  AmigaOS3MixerManager *_mixerManager;

  /**
   * The event source we use for obtaining SDL events.
   */
  AmigaOS3EventSource *_eventSource;


  Backends::Log::Log *_debugLogger;
  Backends::Log::Log *_warningLogger;
  Backends::Log::Log *_errorLogger;


  int _audioThreadPriority;



  /** Hardware window */
  struct Window *_hardwareWindow;

  /** Hardware screen */
  struct Screen *_hardwareScreen;





  /** Unseen game screen */
  Graphics::Surface _screen;
  Graphics::Surface _tmpscreen;




  // Loading screen.
  //Graphics::Surface* _splashSurface;

  Graphics::Surface _overlayscreen8;
  Graphics::Surface _overlayscreen16;
  bool _overlayVisible;
  Graphics::PixelFormat _overlayFormat;
  byte *_overlayColorMap;

  enum {
    kTransactionNone = 0,
    kTransactionActive = 1,
    kTransactionRollback = 2
  };

  struct TransactionDetails {
    bool sizeChanged;
  };
  TransactionDetails _transactionDetails;

  struct VideoState {
    bool setup;

    //bool aspectRatioCorrectionRequested;
    //bool aspectRatioCorrectionInUse;

    int mode;

    uint screenWidth, screenHeight;
    uint overlayScreenHeight;
    //uint scaledHeight;

    uint overlayWidth;
    uint overlayHeight;

    uint bytesPerRow;
    uint overlayBytesPerRow;
  };
  VideoState _videoMode, _oldVideoMode;


  int _transactionMode;

  int _screenChangeCount;

  int _overlayWidth;
  int _overlayHeight;

  bool _screenDirty;
  bool _overlayDirty;
  bool _mouseDirty;


  // Mouse data.
  struct MouseCursor {

    bool visible;

    uint32 keyColor;

    uint w, h;

    // The mouse position.
    uint x, y;

    int hotX, hotY;

    Graphics::Surface surface;
  };

  MouseCursor _mouseCursor;


  struct MouseCursorMask {

    // The mouse position.
    uint x, y;

    // The size of the cursor image.
    uint w, h;

    Graphics::Surface surface;
  };

  MouseCursorMask _mouseCursorMask;




  // Palette data
  byte *_currentPalette;
  byte *_gamePalette;
  byte * _overlayPalette;

  ULONG *_agaPalette;
  uint _paletteDirtyStart, _paletteDirtyEnd;

  // Shake mode
  int _currentShakePos;
  int _newShakePos;


  /**
   * Get the file path where the user configuration
   * of ScummVM will be saved.
   */
  Common::String getDefaultConfigFileName();


  bool loadGFXMode();
  ULONG loadModeId();
  void saveModeId(ULONG modeId);

  struct Screen* createHardwareScreen(uint width, uint height);
  struct Window* createHardwareWindow(uint width, uint height, struct Screen *screen);
  void unloadGFXMode();
  void updatePalette();

  void loadOverlayPalette();
  void loadOverlayColorMap();

  void drawMouse();
  void undrawMouse();

  //UBYTE *scaleScreen();
    
  // TODO - UNIMPLEMENTED
  virtual void displayActivityIconOnOSD(const Graphics::Surface *icon);
};



#endif
