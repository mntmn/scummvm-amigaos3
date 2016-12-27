#include <clib/cybergraphics_protos.h>
#include <pragmas/cybergraphics_pragmas.h>
extern struct Library *CyberGfxBase;
APTR LockBitMapTags(struct BitMap *bm, Tag tag1, ...)
{
 return(LockBitMapTagList(bm, (struct TagItem *) &tag1));
}
