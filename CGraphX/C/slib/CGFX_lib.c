
#ifdef __SASC_650

#include <constructor.h>
#include <proto/exec.h> 

void __regargs __autoopenfail(char *);

extern LONG __CGFXlibversion;

struct Library *CyberGfxBase;
static struct Library *LibBase;

CBMLIB_CONSTRUCTOR(OpenCGFX)

{
 if (CyberGfxBase=LibBase=OpenLibrary("cybergraphics.library",__CGFXlibversion)) return 0;

 __autoopenfail("cybergraphics.library");
 return 1;
}

CBMLIB_DESTRUCTOR(CloseCGFX)

{
 if (LibBase)
  {
   CloseLibrary (LibBase);
   LibBase=CyberGfxBase=NULL;
  }
}

#else
#error SAS/C 6.50 or newer required.
#endif
