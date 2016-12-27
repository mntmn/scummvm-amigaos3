(*------------------------------------------

  :Module.      CyberGraphics.mod
  :Author.      Mario Kemper  [mk]
  :Address.     Geiststrasse 53 , D-59555 Lippstadt
  :EMail.       magick@bundy.lip.owl.de
  :EMail.       magick@uni-paderborn.de
  :Phone.       02941/5509
  :Revision.    R.3
  :Date.        16-Dec-1995
  :Copyright.   Mario Kemper
  :Language.    Oberon-2
  :Translator.  Oberon 3.20 (03.08.94)
  :Contents.    Interface for the CyberGraphics.library
  :Remarks.     Straight port from the c-includes
  :History.     .0     [mk] 29-Nov-1995 : Initial release
  :History.     .1     [mk] 09-Dec-1995 : dpmsON -> dpmsOn (Nils Knop)
  :History.     .2     [mk] 13-Dec-1995 : added mAttrIsLinearMem
  :History.     .3     [mk] 16-Dec-1995 : addes Tags for LockBitMapTagList()


--------------------------------------------*)



MODULE CyberGraphics;

IMPORT int:=Intuition,u:=Utility,gfx:=Graphics,e:=Exec;

(*                                                          *)
(*  Definition of CyberModeNode (Returned in AllocModeList) *)
(*                                                          *)

TYPE
  CyberModeNodePtr *= UNTRACED POINTER TO CyberModeNode;

  CyberModeNode    *=
    STRUCT (node *: e.Node)
      modeText       *: ARRAY  gfx.displayNameLen OF CHAR;
      displayID      *: LONGINT;
      width          *: INTEGER;
      height         *: INTEGER;
      depth          *: INTEGER;
      displayTagList *: e.APTR;  (* taglist with extended ModeID information *)
    END;

(*                                  *)
(* Parameters for GetCyberMapAttr() *)
(*                                  *)

CONST

  mAttrXMod        *= u.user + 1; (* function returns BytesPerRow if its called with this parameter *)
  mAttrBPPix       *= u.user + 2; (* BytesPerPixel shall be returned *)
  mAttrDispAdr     *= u.user + 3; (* do not use this ! private tag *)
  mAttrPixFmt      *= u.user + 4; (* the pixel format is returned *)
  mAttrWidth       *= u.user + 5; (* returns width in pixels *)
  mAttrHeight      *= u.user + 6; (* returns height in lines *)
  mAttrDepth       *= u.user + 7; (* returns bits per pixel *)
  mAttrIsCyberGfx  *= u.user + 8; (* returns -1 if supplied bitmap is a cybergfx one *)
  mAttrIsLinearMem *= u.user + 9; (* returns -1 if supplied bitmap is linear accessible *)

(*                                 *)
(* Parameters for GetCyberidAttr() *)
(*                                 *)

  idAttrPixFmt   *= u.user + 1; (* the pixel format is returned *)
  idAttrWidth    *= u.user + 2; (* returns visible width in pixels *)
  idAttrHeight   *= u.user + 3; (* returns visible height in lines *)
  idAttrDepth    *= u.user + 4; (* returns bits per pixel *)
  idAttrBPPix    *= u.user + 5; (* BytesPerPixel shall be returned *)

(*                              *)
(* Tags for CyberModeRequest()  *)
(*
                              *)

  mReqTB         *= u.user + 40000H;

(*            *)
(* FilterTags *)
(*            *)

  mReqMinDepth    *= mReqTB+0;  (* Minimum depth for displayed screenmode *)
  mReqMaxDepth    *= mReqTB+1;  (* Maximum depth  "       "        " *)
  mReqMinWidth    *= mReqTB+2;  (* Minumum width  "       "        " *)
  mReqMaxWidth    *= mReqTB+3;  (* Maximum width  "       "        " *)
  mReqMinHeight   *= mReqTB+4;  (* Minumum height "       "        " *)
  mReqMaxHeight   *= mReqTB+5;  (* Minumum height "       "        " *)
  mReqCModelArray *= mReqTB+6;

  mReqWinTitle    *= mReqTB+20;
  mReqOKText      *= mReqTB+21;
  mReqCancelText  *= mReqTB+22;

  mReqScreen      *= mReqTB+30;  (* Screen you wish the Requester to open on *)

(*                            *)
(* Tags for BestCyberModeID() *)
(*                            *)

  bIDTGTB *= u.user+50000H;

(* FilterTags *)

  bIDTGDepth         *= bIDTGTB+0;
  bIDTGNominalWidth  *= bIDTGTB+1;
  bIDTGNominalHeight *= bIDTGTB+2;
  bIDTGMonitorID     *= bIDTGTB+3;

(*                                    *)
(* definition of divers pixel formats *)
(*                                    *)

  pixFmtLUT8     *=  0;
  pixFmtRGB15    *=  1;
  pixFmtBGR15    *=  2;
  pixFmtRGB15PC  *=  3;
  pixFmtBGR15PC  *=  4;
  pixFmtRGB16    *=  5;
  pixFmtBGR16    *=  6;
  pixFmtRGB16PC  *=  7;
  pixFmtBGR16PC  *=  8;
  pixFmtRGB24    *=  9;
  pixFmtBGR24    *= 10;
  pixFmtARGB32   *= 11;
  pixFmtBGRA32   *= 12;
  pixFmtRGBA32   *= 13;

(*                                                        *)
(* SrcRectangle formats defines for xxxPixelArray calls() *)
(*                                                        *)

   rectFmtRGB   *= 0;
   rectFmtRGBA  *= 1;
   rectFmtARGB  *= 2;
   rectFmtLUT8  *= 3;
   rectFmtGREY8 *= 4;

(*                                    *)
(* Parameters for CVideoCtrlTagList() *)
(*                                    *)

   setVCDPMSLevel *= 88002001H;

   dpmsOn       * = 0;   (* Full operation *)
   dpmsStandby  * = 1;   (* Optional state of minimal power reduction *)
   dpmsSuspend  * = 2;   (* Significant reduction of power consumption *)
   dpmsOff      * = 3;   (* Lowest level of power consumption *)

(*                              *)
(* Tags for LockBitMapTagList() *)
(*                              *)

   bmiTB           *= u.user + 4001000H;

   bmiWidth        *= bmiTB + 1;
   bmiHeight       *= bmiTB + 2;
   bmiDepth        *= bmiTB + 3;
   bmiPixFmt       *= bmiTB + 4;
   bmiBytesPerPix  *= bmiTB + 5;
   bmiBytesPerRow  *= bmiTB + 6;
   bmiBaseAddress  *= bmiTB + 7;



   cgfxVersion *= 40;
   cgfxName    *= "cybergraphics.library";

VAR

  cgfx * : e.APTR;

(**--- functions in V40 or higher (Release 40.40) ---*)
PROCEDURE cgfxPrivate1          *{cgfx,- 30}; (*private*)
PROCEDURE cgfxPrivate2          *{cgfx,- 36}; (*private*)
PROCEDURE cgfxPrivate3          *{cgfx,- 42}; (*private*)
PROCEDURE cgfxPrivate4          *{cgfx,- 48}; (*private*)
PROCEDURE cgfxPrivate5          *{cgfx,- 84}; (*private*)
PROCEDURE cgfxPrivate6          *{cgfx,-138}; (*private*)

PROCEDURE AllocCModeListTagList *{cgfx,- 72}(modeListTagList{9}   : ARRAY OF u.TagItem) : CyberModeNodePtr;


PROCEDURE AllocCModeListTags    *{cgfx,- 72}(modeListTags{9}.. : u.Tag) : CyberModeNodePtr;

PROCEDURE BestCModeIDTagList    *{cgfx,- 60}(bestModeIDTagList{8} : ARRAY OF u.TagItem):LONGINT;

PROCEDURE BestCModeIDTags       *{cgfx,- 60}(bestModeIDTags{8}..  : u.Tag):LONGINT;

PROCEDURE CModeRequestTagList   *{cgfx,- 66}(modeRequest{8} : e.APTR;
                                                     modeRequestTagList{9}  : ARRAY OF u.TagItem):LONGINT;

PROCEDURE CModeRequestTags      *{cgfx,- 66}(modeRequest{8}       : e.APTR;
                                                     modeRequestTags{9}.. : u.Tag):LONGINT;

PROCEDURE CVideoCtrlTagList     *{cgfx,-162}(viewPort{8} : gfx.ViewPortPtr;
                                                     tagList{9}  : ARRAY OF u.TagItem);

PROCEDURE CVideoCtrlTags        *{cgfx,-162}(viewPort{8} : gfx.ViewPortPtr;
                                                     tags{9}..   : u.Tag);

PROCEDURE DoCDrawMethodTagList  *{cgfx,-156}(hook{8} : u.HookPtr;
                                                     rastPort{9} :gfx.RastPortPtr;
                                                     tagList{10} : ARRAY OF u.TagItem);

PROCEDURE DoCDrawMethodTags     *{cgfx,-156}(hook{8} : u.HookPtr;
                                                     rastPort{9} :gfx.RastPortPtr;
                                                     tags{10}.. : u.Tag);

PROCEDURE FillPixelArray        *{cgfx,-150}(rastPort{9} : gfx.RastPortPtr;
                                                     destX{0},destY{1},sizeX{2},sizeY{3}:INTEGER;
                                                     argb{4}:LONGINT):LONGINT;

PROCEDURE FreeCModeList         *{cgfx,- 78}(modeList{8} : CyberModeNodePtr);

PROCEDURE GetCyberIDAttr        *{cgfx,-102}(cyberIDAttr{0}:LONGINT;
                                                     cyberDisplayModeID{1}:LONGINT):LONGINT;

PROCEDURE GetCyberMapAttr       *{cgfx,- 96}(cyberGfxBitmap{8}:gfx.BitMapPtr;
                                                     cyberAttrTag{0}:LONGINT):LONGINT;

PROCEDURE InvertPixelArray      *{cgfx,-144}(rastPort{8}:gfx.RastPortPtr;
                                                     destX{0},destY{1},sizeX{2},sizeY{3}:INTEGER):LONGINT;

PROCEDURE IsCyberModeID         *{cgfx,- 54}(displayID{0}:LONGINT):BOOLEAN;

PROCEDURE MovePixelArray        *{cgfx,-132}(srcX{0},srcY{1}:INTEGER;
                                                     rastPort{9}:gfx.RastPortPtr;
                                                     destX{2},destY{3},sizeX{4},sizeY{5}:INTEGER):LONGINT;

PROCEDURE ReadPixelArray        *{cgfx,-120}(destRect{8}:e.APTR;
                                                     destX{0},destY{1},destMod{2}:INTEGER;
                                                     rastPort{9}:gfx.RastPortPtr;
                                                     srcX{3},srcY{4},sizeX{5},sizeY{6}:INTEGER;
                                                     destFormat{7}:LONGINT):LONGINT;

PROCEDURE ReadRGBPixel          *{cgfx,-108}(rastPort{9}:gfx.RastPortPtr;
                                                     x{0},y{1}:INTEGER):LONGINT;

PROCEDURE ScalePixelArray       *{cgfx,- 90}(srcRect{8}:e.APTR;
                                                     srcW{0},srcH{1},srcMod{2}:INTEGER;
                                                     rastPort{9}:gfx.RastPortPtr;
                                                     destX{3},destY{4},destW{5},destH{6}:INTEGER;
                                                     srcFormat{7}:LONGINT):LONGINT;

PROCEDURE WritePixelArray       *{cgfx,-126}(srcRect{8}:e.APTR;
                                                     srcX{0},srcY{1},srcMod{2}:INTEGER;
                                                     rastPort{9}:gfx.RastPortPtr;
                                                     destX{3},destY{4},sizeX{5},sizeY{6}:INTEGER;
                                                     srcFormat{7}:LONGINT):LONGINT;

PROCEDURE WriteRGBPixel         *{cgfx,-114}(rastPort{9}:gfx.RastPortPtr;
                                                     x{0},y{1}:INTEGER;
                                                     argb{2}:LONGINT):LONGINT;

(*--- functions in V40 or higher (Release 40.60) --- *)
PROCEDURE LockBitMapTagList     *{cgfx,-168}(bitMap{8}:gfx.BitMapPtr;
                                                     tagList{9} : ARRAY OF u.TagItem): e.APTR;
PROCEDURE LockBitMapTags        *{cgfx,-168}(bitMap{8}:gfx.BitMapPtr;
                                                     tags{9}.. : u.Tag) :e.APTR;


PROCEDURE UnLockBitMap          *{cgfx,-174}(handle{8}:e.APTR);

BEGIN
  cgfx := e.OpenLibrary(cgfxName,cgfxVersion);
  IF cgfx = NIL THEN
    IF int.DisplayAlert(0,"\x00\x64\x14missing cybergraphics.library\o\o",50) THEN END;
    HALT(20)
  END;

CLOSE
  IF cgfx # NIL THEN e.CloseLibrary(cgfx) END;
END CyberGraphics.


