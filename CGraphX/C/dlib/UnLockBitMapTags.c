#include <clib/cybergraphics_protos.h>
#include <pragmas/cybergraphics_pragmas.h>
extern struct Library *CyberGfxBase;
APTR UnLockBitMapTags(APTR Handle, Tag tag1, ...)
{
 return(UnLockBitMapTagList(Handle, (struct TagItem *) &tag1));
}
