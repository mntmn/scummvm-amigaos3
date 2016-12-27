
#include <exec/memory.h>
#include <intuition/intuitionbase.h>
#include <cybergraphx/cybergraphics.h>  /* v41 uses cybergraphx/ dir */

#ifdef __SASC_60
#include <proto/cybergraphics.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#else
#include <clib/cybergraphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#ifndef _DCC
#include <inline/cybergraphics.h>
#endif
#endif

#include <stdlib.h>

#define CYBERGFXVERSION  41L

#define SCREEN_WANTED_WIDTH  640L
#define SCREEN_WANTED_HEIGHT 480L
#define SCREEN_WANTED_DEPTH   24L
#define SCREEN_MIN_DEPTH      15L

#define IMAGE_WIDTH  256L
#define IMAGE_HEIGHT 256L
#define IMAGE_BPP    3L

#ifdef __SASC_60
#include <string.h>
#define MIN(a,b) __builtin_min(a,b)
#else
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#define SIGMASK(w) (1L<<((w)->UserPort->mp_SigBit))
#define GETIMSG(w) ((struct IntuiMessage *)GetMsg((w)->UserPort))

#ifdef __SASC_650
/* use SAS/C 6.5x's autoinit feature */

LONG __oslibversion   = 39L;
LONG __CGFXlibversion = CYBERGFXVERSION;
#else
struct IntuitionBase *IntuitionBase;
struct Library *CyberGfxBase;
#endif

#ifdef __SASC_650
char Version[] = "$VER: Window 0.4 " __AMIGADATE__ ;
#else
#ifdef _DCC
char Version[] = "$VER: Window 0.4 (" __COMMODORE_DATE__  ")";
#else
char Version[] = "$VER: Window 0.4 (" __DATE__  ")";
#endif
#endif

/*

        display error message

*/

void ErrorF(char *Format,...)

{
 void *Data;

 Data=&Format+1L;
 if (((struct Process *)FindTask(NULL))->pr_CLI)
  {
   (void)VPrintf(Format,Data);
   (void)FPutC(Output(),'\n');
   (void)Flush(Output());
  }
 else
  {
   struct EasyStruct EasyStruct;
   ULONG IDCMPFlags;

   EasyStruct.es_StructSize=sizeof(struct EasyStruct);
   EasyStruct.es_Flags=0L;
   EasyStruct.es_Title="Error";
   EasyStruct.es_TextFormat=Format;
   EasyStruct.es_GadgetFormat="Ok";

   IDCMPFlags=0L;
   (void)EasyRequestArgs(NULL,&EasyStruct,&IDCMPFlags,Data);
  }
}

/*

        create image data

*/

void CreateImageData(UBYTE *Image,ULONG Width,ULONG Height)

{
 ULONG X,Y;

 for (Y=0; Y<Height; Y++)
  for (X=0; X<Width; X++)
   {
    *Image++=(UBYTE)X; /* Red   */
    *Image++=(UBYTE)Y; /* Green */
    *Image++=0;        /* Blue  */
   }
}

/*

        window handling

*/

LONG InnerWidth(struct Window *Window)

{
 return Window->Width-Window->BorderLeft-Window->BorderRight;
}

LONG InnerHeight(struct Window *Window)

{
 return Window->Height-Window->BorderTop-Window->BorderBottom;
}

void RedrawScaleWindow(struct Window *ScaleWindow,UBYTE *ImageData)

{
 (void)ScalePixelArray(ImageData,IMAGE_WIDTH,IMAGE_HEIGHT,
                       IMAGE_WIDTH*IMAGE_BPP,
                       ScaleWindow->RPort,
                       ScaleWindow->BorderLeft,ScaleWindow->BorderTop,
                       InnerWidth(ScaleWindow),
                       InnerHeight(ScaleWindow),
                       RECTFMT_RGB);
}

void RedrawWriteWindow(struct Window *WriteWindow,UBYTE *ImageData)

{
 (void)WritePixelArray(ImageData,0,0,
                       IMAGE_WIDTH*IMAGE_BPP,
                       WriteWindow->RPort,
                       WriteWindow->BorderLeft,WriteWindow->BorderTop,
                       InnerWidth(WriteWindow),
                       InnerHeight(WriteWindow),
                       RECTFMT_RGB);
}

/*

        screen depth fallback

*/

ULONG NextDepth(ULONG Depth)

{
 switch (Depth)
  {
   case 24L: /* 24Bit not available? Try 16Bit!      */
    return 16L;
   case 16L: /* 15Bit not available? Try 15Bit!      */
    return 15L;
   default:  /* Not even 15Bit available? Forget it! */
    return 0L;
  }
}

/*

        main program

*/

int main(int argc, char **argv)

{
 ULONG DisplayID,Depth;
 struct Screen *CyberScreen;
 struct Window *ScaleWindow,*WriteWindow;
 UBYTE *ImageData;
 LONG Done;
 struct IntuiMessage *IntMsg;

#ifndef __SASC_650
 if ((IntuitionBase=(struct IntuitionBase *)
                    OpenLibrary("intuition.library",39L))==NULL) return 20;

 if ((CyberGfxBase=OpenLibrary(CYBERGFXNAME,CYBERGFXVERSION))==NULL)
  {
   CloseLibrary (&IntuitionBase->LibNode);

   ErrorF ("Can't open \"%s\" version %ld or newer.",
           CYBERGFXNAME,CYBERGFXVERSION);
   return 10;
  }
#endif

/* Let CyberGraphX search a display mode for us! */

 Depth=SCREEN_WANTED_DEPTH;
 while (Depth)
  {
   if ((DisplayID=BestCModeIDTags(CYBRBIDTG_NominalWidth,SCREEN_WANTED_WIDTH,
                                  CYBRBIDTG_NominalHeight,SCREEN_WANTED_HEIGHT,
                                  CYBRBIDTG_Depth,Depth,
                                  TAG_DONE))!=INVALID_ID)
    {
/* Because older version of the "cybergraphics.library" don't handle */
/* CYBRBIDTG_Depth properly we query the real depth of the mode.     */

     Depth=GetCyberIDAttr(CYBRIDATTR_DEPTH,DisplayID);
     break;
    }
/* retry with less bits per pixel */
   Depth=NextDepth(Depth);
  }

 if (Depth<SCREEN_MIN_DEPTH)
  {
#ifndef __SASC_650
   CloseLibrary (CyberGfxBase);
   CloseLibrary (&IntuitionBase->LibNode);
#endif

   ErrorF ("Can't find suitable display mode for %ldx%ldx%ld.",
           SCREEN_WANTED_WIDTH,SCREEN_WANTED_HEIGHT,SCREEN_WANTED_DEPTH);
   return 5;
  }

/* open screen, but let Intuition choose the actual dimensions */

 if ((CyberScreen=OpenScreenTags(NULL,
                                 SA_Title,"CyberGraphX Demo",
                                 SA_DisplayID,DisplayID,
                                 SA_Depth,Depth,
                                 TAG_DONE))==NULL)
  {
#ifndef __SASC_650
   CloseLibrary (CyberGfxBase);
   CloseLibrary (&IntuitionBase->LibNode);
#endif

   ErrorF ("Can't open screen.");
   return 5;
  }

/* create Scale window */

 if ((ScaleWindow=OpenWindowTags(NULL,
                                 WA_Title,"Scale",
                                 WA_Flags,WFLG_ACTIVATE|WFLG_SIMPLE_REFRESH|
                                  WFLG_SIZEGADGET|WFLG_RMBTRAP|WFLG_DRAGBAR|
                                  WFLG_DEPTHGADGET|WFLG_CLOSEGADGET,
                                 WA_IDCMP,
                                  IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|
                                  IDCMP_SIZEVERIFY|IDCMP_NEWSIZE,
                                 WA_Left,16,
                                 WA_Top,CyberScreen->BarHeight+16,
                                 WA_Width,IMAGE_WIDTH,
                                 WA_Height,IMAGE_HEIGHT,
                                 WA_CustomScreen,CyberScreen,
                                 TAG_DONE))==NULL)
  {
   CloseScreen (CyberScreen);
#ifndef __SASC_650
   CloseLibrary (CyberGfxBase);
   CloseLibrary (&IntuitionBase->LibNode);
#endif

   ErrorF ("Can't open scale window.");
   return 5;
  }
 (void)WindowLimits(ScaleWindow,
                    ScaleWindow->BorderLeft+ScaleWindow->BorderRight+1,
                    ScaleWindow->BorderTop+ScaleWindow->BorderBottom+1,
                    CyberScreen->Width,CyberScreen->Height);

/* create Write window */

 if ((WriteWindow=OpenWindowTags(NULL,
                                 WA_Title,"Write",
                                 WA_Flags,WFLG_SIMPLE_REFRESH|
                                  WFLG_SIZEGADGET|WFLG_RMBTRAP|WFLG_DRAGBAR|
                                  WFLG_DEPTHGADGET|WFLG_CLOSEGADGET,
                                 WA_IDCMP,
                                  IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|
                                  IDCMP_SIZEVERIFY|IDCMP_NEWSIZE,
                                 WA_Left,CyberScreen->Width-16-IMAGE_WIDTH,
                                 WA_Top,CyberScreen->BarHeight+16,
                                 WA_Width,IMAGE_WIDTH,
                                 WA_Height,IMAGE_HEIGHT,
                                 WA_CustomScreen,CyberScreen,
                                 TAG_DONE))==NULL)
  {
   CloseWindow (ScaleWindow);
   CloseScreen (CyberScreen);
#ifndef __SASC_650
   CloseLibrary (CyberGfxBase);
   CloseLibrary (&IntuitionBase->LibNode);
#endif

   ErrorF ("Can't open write window.");
   return 5;
  }
 (void)WindowLimits(WriteWindow,
                    WriteWindow->BorderLeft+WriteWindow->BorderRight+1,
                    WriteWindow->BorderTop+WriteWindow->BorderBottom+1,
                    MIN(CyberScreen->Width,WriteWindow->BorderLeft+
                        WriteWindow->BorderRight+IMAGE_WIDTH),
                    MIN(CyberScreen->Height,WriteWindow->BorderTop+
                        WriteWindow->BorderBottom+IMAGE_HEIGHT));

/* allocate and create image data */

 if ((ImageData=AllocVec(IMAGE_WIDTH*IMAGE_HEIGHT*IMAGE_BPP,MEMF_PUBLIC))==NULL)
  {
   CloseWindow (WriteWindow);
   CloseWindow (ScaleWindow);
   CloseScreen (CyberScreen);
#ifndef __SASC_650
   CloseLibrary (CyberGfxBase);
   CloseLibrary (&IntuitionBase->LibNode);
#endif

   ErrorF ("Out of memory.");
   return 5;
  }
 CreateImageData (ImageData,IMAGE_WIDTH,IMAGE_HEIGHT);

/* event loop */

 RedrawScaleWindow (ScaleWindow,ImageData);
 RedrawWriteWindow (WriteWindow,ImageData);

 Done=FALSE;
 while (!Done)
  {
   (void)Wait(SIGMASK(WriteWindow)|SIGMASK(ScaleWindow));

   while (IntMsg=GETIMSG(ScaleWindow))
    {
     switch (IntMsg->Class)
      {
       case IDCMP_REFRESHWINDOW:
        BeginRefresh (ScaleWindow);
        RedrawScaleWindow (ScaleWindow,ImageData);
        EndRefresh (ScaleWindow,TRUE);
        break;
       case IDCMP_NEWSIZE:
        RedrawScaleWindow (ScaleWindow,ImageData);
        break;
       case IDCMP_CLOSEWINDOW:
        Done=TRUE;
      }

     ReplyMsg (&IntMsg->ExecMessage);
    }

   while (IntMsg=GETIMSG(WriteWindow))
    {
     switch (IntMsg->Class)
      {
       case IDCMP_REFRESHWINDOW:
        BeginRefresh (WriteWindow);
        RedrawWriteWindow (WriteWindow,ImageData);
        EndRefresh (WriteWindow,TRUE);
        break;
       case IDCMP_NEWSIZE:
        RedrawWriteWindow (WriteWindow,ImageData);
        break;
       case IDCMP_CLOSEWINDOW:
        Done=TRUE;
      }

     ReplyMsg (&IntMsg->ExecMessage);
    }
  }

/* cleanup */

 FreeVec (ImageData);
 CloseWindow (WriteWindow);
 CloseWindow (ScaleWindow);
 CloseScreen (CyberScreen);
#ifndef __SASC_650
 CloseLibrary (CyberGfxBase);
 CloseLibrary (&IntuitionBase->LibNode);
#endif

 return 0;
}
