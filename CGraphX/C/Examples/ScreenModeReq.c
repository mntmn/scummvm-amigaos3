
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <intuition/intuitionbase.h>
#include <libraries/asl.h>
#include <cybergraphx/cybergraphics.h>          /* v41 uses cybergraphx/ dir */

#ifdef __SASC_60
#include <proto/asl.h>
#include <proto/cybergraphics.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#else
#include <clib/asl_protos.h>
#include <clib/cybergraphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#endif

#include <stdlib.h>

#define CYBERGFXNAME     "cybergraphics.library"
#define CYBERGFXVERSION  41L

#ifdef __SASC_650
/* use SAS/C 6.5x's autoinit feature */

LONG __oslibversion   = 39L;
LONG __CGFXlibversion = CYBERGFXVERSION;
#else
struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;
struct Library *AslBase,*CyberGfxBase;
#endif

#ifdef __SASC_650
char Version[] = "$VER: ScreenModeReq 0.1 " __AMIGADATE__ ;
#else
char Version[] = "$VER: ScreenModeReq 0.1 (" __COMMODORE_DATE__  ")";
#endif

char ResultFormat[] = "You selected screen mode \"%s\".";

/*

        display error message

*/

void ErrorF(char *Format,...)

{
 void *Data;

 Data=&Format+1L;
 if (Cli())
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

        our filter hook function

*/

#ifdef __SASC_60
LONG __asm __interrupt __saveds
     SMFilterFunc(register __a0 struct Hook *Hook,
                  register __a1 ULONG DisplayID,
                  register __a2 struct ScreenModeRequester *SMReq)

{
/* filter all native graphics modes */
 if (!IsCyberModeID(DisplayID)) return FALSE;

/* filter all modes with pixelformat PIXFMT_LUT8 */
 return (GetCyberIDAttr(CYBRIDATTR_PIXFMT,DisplayID)!=PIXFMT_LUT8);
}
#else
LONG __geta4
     SMFilterFunc(__a0 struct Hook *Hook,
                  __a1 ULONG DisplayID,
                  __a2 struct ScreenModeRequester *SMReq)

{
/* filter all native graphics modes */
 if (!IsCyberModeID(DisplayID)) return FALSE;

/* filter all modes with pixelformat PIXFMT_LUT8 */
 return (GetCyberIDAttr(CYBRIDATTR_PIXFMT,DisplayID)!=PIXFMT_LUT8);
}
#endif

struct Hook SMFilterHook = {NULL,NULL,(void *)SMFilterFunc,NULL,NULL};

/*

        main program

*/

int main(int argc, char **argv)

{
 struct ScreenModeRequester *SMReq;

#ifndef __SASC_650
 if ((GfxBase=(struct GfxBase *)
              OpenLibrary("graphics.library",39L))==NULL) return 10;

 if ((IntuitionBase=(struct IntuitionBase *)
                    OpenLibrary("intuition.library",39L))==NULL)
  {
   CloseLibrary (&GfxBase->LibNode);

   return 10;
  }

 if ((AslBase=OpenLibrary(AslName,39L))==NULL)
  {
   CloseLibrary (&IntuitionBase->LibNode);
   CloseLibrary (&GfxBase->LibNode);

   return 20;
  }

 if ((CyberGfxBase=OpenLibrary(CYBERGFXNAME,CYBERGFXVERSION))==NULL)
  {
   CloseLibrary (AslBase);
   CloseLibrary (&IntuitionBase->LibNode);
   CloseLibrary (&GfxBase->LibNode);

   ErrorF ("Can't open \"%s\" version %ld or newer.",
           CYBERGFXNAME,CYBERGFXVERSION);
   return 10;
  }
#endif

 if ((SMReq=AllocAslRequestTags(ASL_ScreenModeRequest,
                                ASLSM_TitleText,"CyberGraphX",
                                ASLSM_FilterFunc,&SMFilterHook,
                                TAG_DONE))==NULL)
  {
#ifndef __SASC_650
   CloseLibrary (CyberGfxBase);
   CloseLibrary (AslBase);
   CloseLibrary (&IntuitionBase->LibNode);
   CloseLibrary (&GfxBase->LibNode);
#endif

   ErrorF ("Can't create screen mode requester\n");
   return 10;
  }

 if (AslRequestTags(SMReq,TAG_DONE))
  {
   struct NameInfo NameInfo;

   if (GetDisplayInfoData(NULL,
                          (UBYTE *)&NameInfo,
                          sizeof(struct NameInfo),
                          DTAG_NAME,
                          SMReq->sm_DisplayID))
    {
     char *Ptr;

     Ptr=NameInfo.Name;
     if (Cli())
      {
       (void)VPrintf(ResultFormat,&Ptr);
       (void)FPutC(Output(),'\n');
       (void)Flush(Output());
      }
     else
      {
       struct EasyStruct EasyStruct;
       ULONG IDCMPFlags;

       EasyStruct.es_StructSize=sizeof(struct EasyStruct);
       EasyStruct.es_Flags=0L;
       EasyStruct.es_Title="Result";
       EasyStruct.es_TextFormat=ResultFormat;
       EasyStruct.es_GadgetFormat="Ok";

       IDCMPFlags=0L;
       (void)EasyRequestArgs(NULL,&EasyStruct,&IDCMPFlags,&Ptr);
      }
    }
  }

 FreeAslRequest (SMReq);
#ifndef __SASC_650
 CloseLibrary (CyberGfxBase);
 CloseLibrary (AslBase);
 CloseLibrary (&IntuitionBase->LibNode);
 CloseLibrary (&GfxBase->LibNode);
#endif

 return 0;
}
