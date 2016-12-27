(*------------------------------------------

  :Program.     Window
  :Author.      Mario Kemper  [mk]
  :Address.     Geiststrasse 53 , D-59555 Lippstadt
  :EMail.       magick@bundy.lip.owl.de
  :EMail.       magick@uni-paderborn.de
  :Phone.       02941/5509
  :Version.     V0.1
  :Date.        30-Nov-1995
  :Copyright.   Mario Kemper
  :Language.    Oberon-2
  :Translator.  Oberon 3.20 (03.08.94)
  :Contents.    Demo for the Cybergraphics.library
  :Remarks.     Original C-Source by Matthias Scheler (Tron)
  :Remarks.     This is a rather clumsy straight port from the C-Example
  :Usage.       Runs only from CLI (sorry, but i'm really lazy :-)
  :History.     0.1    [mk] 30-Nov-1995 : Initial Release

--------------------------------------------*)


MODULE Window;

IMPORT cgfx:=CyberGraphics,ol:=OberonLib,int:=Intuition,e:Exec,d:Dos,
       s:=SYSTEM,u:=Utility,gfx:=Graphics,NoGuru;

(* SHORTSET, because SHORTINTS only go from -127 to 127 *)
TYPE DataPtr = UNTRACED POINTER TO ARRAY (MAX(LONGINT) -1) OF CHAR;


CONST screenWantedWidth  = 640;
      screenWantedHeight = 480;
      screenWantedDepth  =  24;
      screenMinDepth     =  15;
      imageWidth         = 256;
      imageHeight        = 256;
      imageBpp           =   3;


PROCEDURE Min(a,b:INTEGER) : INTEGER;
BEGIN
  IF a<b THEN
    RETURN(a);
  ELSE
    RETURN(b);
  END;
END Min;

PROCEDURE Next(x:CHAR):CHAR;
(* return the next character or CHR(0), if ORD(x) > 255 *)
(* this is needed, because a Shortint only ranges from -127 to 127*)
BEGIN
  IF ORD(x) < 255 THEN
    x:=CHR(ORD(x)+1);
  ELSE
    x:=CHR(0);
  END;
  RETURN(x);
END Next;

PROCEDURE CreateImageData(image : DataPtr;width,height:LONGINT);

VAR x,y,i:LONGINT;
    r,g:CHAR;


BEGIN
  i:=0;r:=CHR(0);g:=CHR(0);
  FOR y:=0 TO height-1 DO
    FOR x:=0 TO width-1 DO
      image[i] := r; (*red*)
      INC(i);
      image[i] := g; (*green*)
      INC(i);
      image[i] := CHR(0); (*blue*)
      INC(i);
      r:=Next(r);
    END;
    g:=Next(g);
  END;
END CreateImageData;


(* window handling *)

PROCEDURE InnerWidth (window : int.WindowPtr):LONGINT;
BEGIN
  RETURN(window.width - window.borderLeft - window.borderRight);
END InnerWidth;

PROCEDURE InnerHeight (window : int.WindowPtr):LONGINT;
BEGIN
  RETURN(window.height - window.borderTop - window.borderBottom);
END InnerHeight;


PROCEDURE RedrawScaleWindow (scaleWindow : int.WindowPtr; imageData : DataPtr);
VAR long:LONGINT;

BEGIN
  long:=cgfx.ScalePixelArray(imageData,imageWidth,imageHeight,imageWidth*imageBpp,
                       scaleWindow.rPort,scaleWindow.borderLeft,scaleWindow.borderTop,
                       SHORT(InnerWidth(scaleWindow)), SHORT(InnerHeight(scaleWindow)),
                       cgfx.rectFmtRGB);
END RedrawScaleWindow;


PROCEDURE RedrawWriteWindow(writeWindow : int.WindowPtr;imageData : DataPtr);
VAR long:LONGINT;

BEGIN
  long:=cgfx.WritePixelArray(imageData,0,0,imageWidth*imageBpp,
                             writeWindow.rPort,writeWindow.borderLeft,writeWindow.borderTop,
                             SHORT(InnerWidth(writeWindow)),
                             SHORT(InnerHeight(writeWindow)),
                             cgfx.rectFmtRGB);
END RedrawWriteWindow;

(* screen depth fallback *)

PROCEDURE NextDepth (depth : LONGINT):LONGINT;
VAR
result:LONGINT;

BEGIN
  CASE depth OF
  |24 : result:=16;
  |16 : result:=15;
  ELSE
   result:=0;
  END;
  RETURN(result);
END NextDepth;

(* main program *)

VAR
  displayID,depth : LONGINT;
  cyberScreen     : int.ScreenPtr;
  scaleWindow,
  writeWindow     : int.WindowPtr;
  imageData       : DataPtr;
  done            : BOOLEAN;
  signal          : LONGSET;
  bool            : BOOLEAN;
  intMsg          : int.IntuiMessagePtr;
BEGIN
  done := FALSE;
  depth := screenWantedDepth;
  WHILE ((depth # 0) AND (NOT done)) DO
    displayID:=cgfx.BestCModeIDTags(cgfx.bIDTGNominalWidth,screenWantedWidth,
                                    cgfx.bIDTGNominalHeight,screenWantedHeight,
                                    cgfx.bIDTGDepth,depth,
                                    u.done);
    IF displayID # gfx.invalidID THEN
      depth:=cgfx.GetCyberIDAttr(cgfx.idAttrDepth,displayID);
      done:=TRUE;
    ELSE
      depth:=NextDepth(depth);
    END;

  END (*WHILE*);

  IF depth < screenMinDepth THEN
    d.PrintF("Can't find suitable display mode for %ldx%ldx%ld.",
             screenWantedWidth,screenWantedHeight,screenWantedDepth);
    HALT(5);

  END;

(* open screen, but let intuition choose the actual dimensions *)

  cyberScreen:=int.OpenScreenTagsA(NIL,int.saTitle,s.ADR("CyberGraphX Demo"),
                                  int.saDisplayID,displayID,
                                  int.saDepth,depth,u.done);
  IF cyberScreen = NIL THEN
    d.PrintF("Can't open screen");
    HALT(5);
  END;

  (* create scale Window *)

  scaleWindow:=int.OpenWindowTagsA(NIL,
                                  int.waTitle,s.ADR("Scale"),
                                  int.waFlags,LONGSET{int.activate,int.simpleRefresh,
                                    int.windowSizing,int.rmbTrap,int.windowDrag,
                                    int.windowDepth,int.windowClose},
                                  int.waIDCMP,LONGSET{
                                   int.closeWindow,int.refreshWindow,
                                   int.sizeVerify,int.newSize},
                                  int.waLeft,16,
                                  int.waTop,cyberScreen.barHeight+16,
                                  int.waWidth,imageWidth,
                                  int.waHeight,imageHeight,
                                  int.waCustomScreen,cyberScreen,
                                  u.done);

  IF scaleWindow = NIL THEN
    d.PrintF("Can't open scale window.");
    HALT(5);
  END;

  IF int.WindowLimits(scaleWindow,
                      scaleWindow.borderLeft+scaleWindow.borderRight+1,
                      scaleWindow.borderTop+scaleWindow.borderBottom+1,
                      cyberScreen.width,cyberScreen.height) THEN
  END;

  (* create Write window *)

  writeWindow:=int.OpenWindowTagsA(NIL,
                                  int.waTitle,s.ADR("Write"),
                                  int.waFlags,LONGSET{int.activate,int.simpleRefresh,
                                    int.windowSizing,int.rmbTrap,int.windowDrag,
                                    int.windowDepth,int.windowClose},
                                  int.waIDCMP,LONGSET{
                                   int.closeWindow,int.refreshWindow,
                                   int.sizeVerify,int.newSize},
                                  int.waLeft,cyberScreen.width-16-imageWidth,
                                  int.waTop,cyberScreen.barHeight+16,
                                  int.waWidth,imageWidth,
                                  int.waHeight,imageHeight,
                                  int.waCustomScreen,cyberScreen,
                                  u.done);

  IF writeWindow = NIL THEN
    d.PrintF("Can't open write window.");
    HALT(5);
  END;

  IF int.WindowLimits(writeWindow,
                      writeWindow.borderLeft+writeWindow.borderRight+1,
                      writeWindow.borderTop+writeWindow.borderBottom+1,
                      Min(cyberScreen.width,writeWindow.borderLeft+
                          writeWindow.borderRight+imageWidth),
                      Min(cyberScreen.height,writeWindow.borderTop+
                          writeWindow.borderBottom+imageHeight)) THEN
  END;

  (* allocate and create image data *)

  imageData:=e.AllocVec(imageWidth*imageHeight*imageBpp, LONGSET{e.public});
  IF imageData = NIL THEN
    d.PrintF("out of memory.");
    HALT(5);
  END;

  CreateImageData (imageData,imageWidth,imageHeight);

(* event loop *)

  RedrawScaleWindow(scaleWindow,imageData);
  RedrawWriteWindow(writeWindow,imageData);

  done:=FALSE;

  WHILE(NOT done) DO
    signal:=e.Wait(LONGSET{scaleWindow.userPort.sigBit,writeWindow.userPort.sigBit});
    intMsg:=e.GetMsg(scaleWindow.userPort);
    WHILE (intMsg#NIL) DO
      IF int.refreshWindow IN intMsg.class THEN
         int.BeginRefresh(scaleWindow);
         RedrawScaleWindow(scaleWindow,imageData);
         int.EndRefresh(scaleWindow,int.LTRUE);
      END;
      IF int.newSize IN intMsg.class THEN
         RedrawScaleWindow(scaleWindow,imageData);
      END;
      IF int.closeWindow IN intMsg.class THEN
         done:=TRUE;
      END;

      e.ReplyMsg(intMsg);
      intMsg:=e.GetMsg(scaleWindow.userPort);
    END (*WHILE*);

    intMsg:=e.GetMsg(writeWindow.userPort);

    WHILE (intMsg#NIL) DO

      IF int.refreshWindow IN intMsg.class THEN
         int.BeginRefresh(writeWindow);
         RedrawWriteWindow(writeWindow,imageData);
         int.EndRefresh(writeWindow,int.LTRUE);
      END;
      IF int.newSize IN intMsg.class THEN
         RedrawWriteWindow(writeWindow,imageData);
      END;

      IF int.closeWindow IN intMsg.class THEN
         done:=TRUE;
      END;
      e.ReplyMsg(intMsg);
      intMsg:=e.GetMsg(writeWindow.userPort);
    END (*WHILE *);

  END (*WHILE*);

  (* CleanUP *)
CLOSE

  IF imageData   # NIL THEN e.FreeVec (imageData) END;
  IF writeWindow # NIL THEN int.CloseWindow(writeWindow) END;
  IF scaleWindow # NIL THEN int.CloseWindow (scaleWindow) END;
  IF cyberScreen # NIL THEN bool:=int.CloseScreen (cyberScreen) END;

END Window.
