/*
**  $VER: cybergraphics.h 41.18 (21.02.1998)
**
**  include file for cybergraphics.library
**
**  Copyright © 1996-1998 by phase5 digital products
**      All Rights reserved.
**
*/

OPT MODULE
OPT PREPROCESS
OPT EXPORT

MODULE 'exec/nodes'
MODULE 'utility/tagitem'
MODULE 'graphics/displayinfo'



#define CYBERGFXNAME  'cybergraphics.library'
CONST CYBERGFX_INCLUDE_VERSION  =41

/*                                                          *
 *  Definition of CyberModeNode Returned in AllocModeList *
 *                                                          */

  OBJECT  cyberModeNode
     node:ln
     modeText[DISPLAYNAMELEN]:ARRAY OF CHAR
     displayID:LONG
     width:INT
     height:INT
     depth:INT
     displayTagList:PTR TO LONG   /* taglist with extended ModeID information */
  ENDOBJECT


/*                                  *
 * Parameters for GetCyberMapAttr() *
 *                                  */

CONST CYBRMATTR_XMOD        =$80000001 /* function returns BytesPerRow IF its called with this parameter */
CONST CYBRMATTR_BPPIX       =$80000002 /* BytesPerPixel shall be returned */
CONST CYBRMATTR_DISPADR     =$80000003 /* DO not use this ! private tag */
CONST CYBRMATTR_PIXFMT      =$80000004 /* the pixel format is returned */
CONST CYBRMATTR_WIDTH       =$80000005 /* returns width in pixels */
CONST CYBRMATTR_HEIGHT      =$80000006 /* returns height in lines */
CONST CYBRMATTR_DEPTH       =$80000007 /* returns bits per pixel */
CONST CYBRMATTR_ISCYBERGFX  =$80000008 /* returns -1 IF supplied bitmap is a cybergfx one */
CONST CYBRMATTR_ISLINEARMEM =$80000009 /* returns -1 IF supplied bitmap is linear accessable */

/*                                 *
 * Parameters for GetCyberIDAttr() *
 *                                 */

CONST CYBRIDATTR_PIXFMT =$80000001 /* the pixel format is returned */
CONST CYBRIDATTR_WIDTH  =$80000002 /* returns visible width in pixels */
CONST CYBRIDATTR_HEIGHT =$80000003 /* returns visible height in lines */
CONST CYBRIDATTR_DEPTH  =$80000004 /* returns bits per pixel */
CONST CYBRIDATTR_BPPIX  =$80000005 /* BytesPerPixel shall be returned */

/*                              *
 * Tags for CyberModeRequest()  *
 *                              */

CONST CYBRMREQ_TB          =TAG_USER+$40000

/*            *
 * FilterTags *
 *            */

CONST CYBRMREQ_MinDepth    =CYBRMREQ_TB+0  /* Minimum depth FOR displayed screenmode */
CONST CYBRMREQ_MaxDepth    =CYBRMREQ_TB+1  /* Maximum depth  "       "        " */
CONST CYBRMREQ_MinWidth    =CYBRMREQ_TB+2  /* Minumum width  "       "        " */
CONST CYBRMREQ_MaxWidth    =CYBRMREQ_TB+3  /* Maximum width  "       "        " */
CONST CYBRMREQ_MinHeight   =CYBRMREQ_TB+4  /* Minumum height "       "        " */
CONST CYBRMREQ_MaxHeight   =CYBRMREQ_TB+5  /* Minumum height "       "        " */
CONST CYBRMREQ_CModelArray =CYBRMREQ_TB+6

CONST CYBRMREQ_WinTitle    =CYBRMREQ_TB+20
CONST CYBRMREQ_OKText      =CYBRMREQ_TB+21
CONST CYBRMREQ_CancelText  =CYBRMREQ_TB+22

CONST CYBRMREQ_Screen      =CYBRMREQ_TB+30  /* Screen you wish the Requester TO open on */

/*                            *
 * Tags for BestCyberModeID() *
 *                            */

CONST CYBRBIDTG_TB  =TAG_USER+$50000

/* FilterTags */

CONST CYBRBIDTG_Depth         =CYBRBIDTG_TB+0
CONST CYBRBIDTG_NominalWidth  =CYBRBIDTG_TB+1
CONST CYBRBIDTG_NominalHeight =CYBRBIDTG_TB+2
CONST CYBRBIDTG_MonitorID     =CYBRBIDTG_TB+3
CONST CYBRBIDTG_BoardName     =CYBRBIDTG_TB+5

/*                                    *
 * definition of divers pixel formats *
 *                                    */

ENUM  PIXFMT_LUT8 =0,
      PIXFMT_RGB15,
      PIXFMT_BGR15,
      PIXFMT_RGB15PC,
      PIXFMT_BGR15PC,
      PIXFMT_RGB16,
      PIXFMT_BGR16,
      PIXFMT_RGB16PC,
      PIXFMT_BGR16PC,
      PIXFMT_RGB24,
      PIXFMT_BGR24,
      PIXFMT_ARGB32,
      PIXFMT_BGRA32,
      PIXFMT_RGBA32

/*                                                        *
 * SrcRectangle formats defines for xxxPixelArray calls() * 
 *                                                        */

ENUM  RECTFMT_RGB=0,
      RECTFMT_RGBA,
      RECTFMT_ARGB,
      RECTFMT_LUT8,
      RECTFMT_GREY8

/*                                    *
 * Parameters for CVideoCtrlTagList() *
 *                                    */

CONST SETVC_DPMSLevel  =$88002001

ENUM  DPMS_ON=0,        /* Full operation                             */
      DPMS_STANDBY,     /* Optional state of minimal power reduction  */
      DPMS_SUSPEND,     /* Significant reduction of power consumption */
      DPMS_OFF          /* Lowest level of power consumption          */

/*                              *
 * Tags for LockBitMapTagList() *
 *                              */

CONST LBMI_WIDTH       =$84001001
CONST LBMI_HEIGHT      =$84001002
CONST LBMI_DEPTH       =$84001003
CONST LBMI_PIXFMT      =$84001004
CONST LBMI_BYTESPERPIX =$84001005
CONST LBMI_BYTESPERROW =$84001006
CONST LBMI_BASEADDRESS =$84001007

/*                                *
 * Tags for UnLockBitMapTagList() *
 *                                */

CONST UBMI_UPDATERECTS  =$85001001
CONST UBMI_REALLYUNLOCK =$85001002



/*                *
 * Message passed to the DoCDrawMethodTagList() hook function *
 *                */


OBJECT cDrawMsg
  cdm_MemPtr:PTR TO LONG
  cdm_offx:LONG
  cdm_offy:LONG
  cdm_xsize:LONG
  cdm_ysize:LONG
  cdm_BytesPerRow:INT
  cdm_BytesPerPix:INT
  cdm_ColorModel:INT
ENDOBJECT

/*              *
 * Colour Table source formats for WriteLUTPixelArray() *
 *              */

CONST CTABFMT_XRGB8 =0 /* ULONG [] table */


/*              *
 *  graphics.library/AllocBitMap() extended flags *
 *              */


CONST BMB_SPECIALFMT =7
#define BMF_SPECIALFMT =Shl(1, BMB_SPECIALFMT)


#define SHIFT_PIXFMT( fmt ) (Shl(fmt)24))




