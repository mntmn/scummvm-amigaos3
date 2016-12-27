#include <clib/cybergraphics_protos.h>
#include <pragmas/cybergraphics_pragmas.h>
extern struct Library *CyberGfxBase;
ULONG BestCModeIDTags(Tag tag1, ...)
{
 return(BestCModeIDTagList((struct TagItem *) &tag1));
}
