OPT PREPROCESS                  ->FOR the define

MODULE 'exec/memory'            ->allocvector
MODULE 'exec/ports'             ->userport.bitmap
MODULE 'cybergraphx/cybergraphics'
MODULE 'cybergraphics'
MODULE 'dos'
MODULE 'intuition', 'intuition/intuition', 'intuition/screens'
MODULE 'utility/tagitem'
MODULE 'graphics/modeid'

#define CYBERGFXNAME     'cybergraphics.library'
CONST CYBERGFXVERSION  =41

CONST SCREEN_WANTED_WIDTH  =640
CONST SCREEN_WANTED_HEIGHT =480
CONST SCREEN_WANTED_DEPTH   =24
CONST SCREEN_MIN_DEPTH      =15

CONST IMAGE_WIDTH  =256
CONST IMAGE_HEIGHT =256
CONST IMAGE_BPP    =3


PROC min(a,b) IS IF a<b THEN a ELSE b

PROC errorF(format:PTR TO CHAR)
  DEF easystruct:easystruct
  DEF data:PTR TO LONG
  DEF idcmpflags

 data:={format}+1
 IF FindTask(NIL)  ->)->pr_CLI)

   Vprintf(format,data)
   FputC(Output(),'\n')
   Flush(Output())
 ELSE

   easystruct.structsize:= SIZEOF easystruct
   easystruct.flags:=0
   easystruct.title:='Error'
   easystruct.textformat:=format
   easystruct.gadgetformat:='Ok'

   idcmpflags:=0
   EasyRequestArgs(NIL, {easystruct}, {idcmpflags}, data)
  ENDIF
ENDPROC

PROC createImageData(image, width,height)
  DEF x,y

 FOR y:=0 TO height-1
  FOR x:=0 TO width-1
    image[]++:=x       /* Red   */
    image[]++:=y       /* Green */
    image[]++:=0       /* Blue  */
  ENDFOR
 ENDFOR
ENDPROC 

PROC innerWidth(window:PTR TO window) IS window.width-window.borderleft-window.borderright

PROC innerHeight(window:PTR TO window) IS window.height-window.bordertop-window.borderbottom

PROC redrawScaleWindow(scalewindow:PTR TO window,imagedata)

 ScalePixelArray(imagedata,IMAGE_WIDTH,IMAGE_HEIGHT,
                       IMAGE_WIDTH*IMAGE_BPP,
                       scalewindow.rport,
                       scalewindow.borderleft,scalewindow.bordertop,
                       innerWidth(scalewindow),
                       innerHeight(scalewindow),
                       RECTFMT_RGB)
ENDPROC

PROC redrawWriteWindow(writewindow:PTR TO window,imagedata)

 WritePixelArray(imagedata,0,0,
                  IMAGE_WIDTH*IMAGE_BPP,
                  writewindow.rport,
                  writewindow.borderleft,writewindow.bordertop,
                  innerWidth(writewindow),
                  innerHeight(writewindow),
                  RECTFMT_RGB)
ENDPROC

PROC nextDepth(depth)

 SELECT depth
   CASE 24; RETURN 16      /* 24Bit not available? Try 16Bit!      */
   CASE 16; RETURN 15     /* 15Bit not available? Try 15Bit!      */
   DEFAULT; RETURN 0     /* Not even 15Bit available? Forget it! */
  ENDSELECT
ENDPROC

PROC main()
 DEF displayID,depth;
 DEF cyberScreen:PTR TO screen
 DEF scalewindow:PTR TO window,writewindow:PTR TO window
 DEF imagedata
 DEF done
 DEF intmsg:PTR TO intuimessage
 DEF error[40]:STRING
 DEF signal
 DEF id_ok= FALSE

 IF (cybergfxbase:=OpenLibrary(CYBERGFXNAME,CYBERGFXVERSION))=NIL
   StringF(error,'Can not open \s version \d OR newer.',CYBERGFXNAME,CYBERGFXVERSION)
   errorF(error)
   RETURN 10
  ENDIF


/* Let CyberGraphX search a display mode for us! */

 depth:=SCREEN_WANTED_DEPTH
  WHILE id_ok= FALSE

    IF ((displayID:=BestCModeIDTagList([CYBRBIDTG_NOMINALWIDTH,SCREEN_WANTED_WIDTH,
                                        CYBRBIDTG_NOMINALHEIGHT,SCREEN_WANTED_HEIGHT,
                                        CYBRBIDTG_DEPTH,depth,
                                        TAG_DONE]))<>INVALID_ID)

/* Because older version of the "cybergraphics.library" don't handle */
/* CYBRBIDTG_Depth properly we query the real depth of the mode.     */

      depth:=GetCyberIDAttr(CYBRIDATTR_DEPTH,displayID)
      id_ok := TRUE

    ELSE
      /* retry with less bits per pixel */
      depth:=nextDepth(depth)
    ENDIF
  ENDWHILE

  IF depth<SCREEN_MIN_DEPTH

    CloseLibrary(cybergfxbase)

    StringF(error,'Can not find suitable display mode FOR /d,/d,/d.',
           SCREEN_WANTED_WIDTH,SCREEN_WANTED_HEIGHT,SCREEN_WANTED_DEPTH)
    errorF(error)
    RETURN 5
  ENDIF

/* open screen, but let Intuition choose the actual dimensions */

 IF ((cyberScreen:=OpenScreenTagList(NIL,
                                 [SA_TITLE,      'CyberGraphX Demo',
                                 SA_DISPLAYID,    displayID,
                                 SA_DEPTH,        depth,
                                 TAG_DONE]))= NIL)


   CloseLibrary(cybergfxbase)


  StringF(error,'Can/at open screen.')
   errorF (error)
   RETURN 5
  ENDIF

/* create Scale window */

 IF ((scalewindow:=OpenWindowTagList(NIL,
                                 [WA_TITLE,          'Scale',
                                 WA_FLAGS,WFLG_ACTIVATE OR WFLG_SIMPLE_REFRESH OR
                                    WFLG_SIZEGADGET OR WFLG_RMBTRAP OR WFLG_DRAGBAR OR
                                    WFLG_DEPTHGADGET OR WFLG_CLOSEGADGET,
                                 WA_IDCMP,
                                    IDCMP_CLOSEWINDOW OR IDCMP_REFRESHWINDOW OR
                                    IDCMP_SIZEVERIFY OR IDCMP_NEWSIZE,
                                 WA_LEFT,         16,
                                 WA_TOP,          cyberScreen.barheight+16,
                                 WA_WIDTH,        IMAGE_WIDTH,
                                 WA_HEIGHT,       IMAGE_HEIGHT,
                                 WA_CUSTOMSCREEN, cyberScreen,
                                 TAG_DONE]))= NIL)

   CloseScreen(cyberScreen)
   CloseLibrary(cybergfxbase)

   StringF(error,'Can\at open scale window.')
   errorF(error)
   RETURN 5
  ENDIF
 WindowLimits(scalewindow,
                    scalewindow.borderleft+scalewindow.borderright+1,
                    scalewindow.bordertop+scalewindow.borderbottom+1,
                    cyberScreen.width,cyberScreen.height)

/* create Write window */

 IF ((writewindow:=OpenWindowTagList(NIL,
                                 [WA_TITLE,      'Write',
                                 WA_FLAGS,        WFLG_SIMPLE_REFRESH OR
                                  WFLG_SIZEGADGET OR WFLG_RMBTRAP OR WFLG_DRAGBAR OR
                                  WFLG_DEPTHGADGET OR WFLG_CLOSEGADGET,
                                 WA_IDCMP,
                                  IDCMP_CLOSEWINDOW OR IDCMP_REFRESHWINDOW OR
                                  IDCMP_SIZEVERIFY OR IDCMP_NEWSIZE,
                                 WA_LEFT,         cyberScreen.width-16-IMAGE_WIDTH,
                                 WA_TOP,          cyberScreen.barheight+16,
                                 WA_WIDTH,        IMAGE_WIDTH,
                                 WA_HEIGHT,       IMAGE_HEIGHT,
                                 WA_CUSTOMSCREEN, cyberScreen,
                                 TAG_DONE]))= NIL)

   CloseWindow(scalewindow)
   CloseScreen(cyberScreen)
   CloseLibrary(cybergfxbase)

   StringF(error,'Can\at open write window.')
   errorF (error)
   RETURN 5
  ENDIF
 WindowLimits(writewindow,
                    writewindow.borderleft+writewindow.borderright+1,
                    writewindow.bordertop+writewindow.borderbottom+1,
                    min(cyberScreen.width,writewindow.borderleft+
                        writewindow.borderright+IMAGE_WIDTH),
                    min(cyberScreen.height,writewindow.bordertop+
                        writewindow.borderbottom+IMAGE_HEIGHT))

/* allocate and create image data */

 IF ((imagedata:=AllocVec(IMAGE_WIDTH*IMAGE_HEIGHT*IMAGE_BPP,MEMF_PUBLIC))= NIL)

   CloseWindow(writewindow)
   CloseWindow(scalewindow)
   CloseScreen(cyberScreen)
   CloseLibrary(cybergfxbase)

    StringF(error,'Out of memory')
   errorF(error)
   RETURN 5
  ENDIF
 createImageData(imagedata,IMAGE_WIDTH,IMAGE_HEIGHT)

/* event loop */

 redrawScaleWindow(scalewindow,imagedata)
 redrawWriteWindow(writewindow,imagedata)

 done:=FALSE
 WHILE Not(done)

  signal:=Wait(Shl(1, scalewindow.userport.sigbit) OR Shl(1, writewindow.userport.sigbit))

   WHILE intmsg:=GetMsg(scalewindow.userport)
     signal:=intmsg.class
     SELECT signal

       CASE IDCMP_REFRESHWINDOW
        BeginRefresh(scalewindow)
        redrawScaleWindow(scalewindow,imagedata)
        EndRefresh(scalewindow,TRUE)

       CASE IDCMP_NEWSIZE
        redrawScaleWindow(scalewindow,imagedata)

       CASE IDCMP_CLOSEWINDOW
        done:=TRUE
      ENDSELECT

     ReplyMsg(intmsg)
    ENDWHILE
   WHILE intmsg:=GetMsg(writewindow.userport)
     signal:=intmsg.class
     SELECT signal

       CASE IDCMP_REFRESHWINDOW
        BeginRefresh(writewindow)
        redrawWriteWindow(writewindow,imagedata)
        EndRefresh(writewindow,TRUE)

       CASE IDCMP_NEWSIZE
        redrawWriteWindow (writewindow,imagedata)

       CASE IDCMP_CLOSEWINDOW
        done:=TRUE
     ENDSELECT

     ReplyMsg(intmsg)
    ENDWHILE
  ENDWHILE
  /* cleanup */

 FreeVec(imagedata)
 CloseWindow(writewindow)
 CloseWindow(scalewindow)
 CloseScreen(cyberScreen)
 CloseLibrary(cybergfxbase)
ENDPROC




