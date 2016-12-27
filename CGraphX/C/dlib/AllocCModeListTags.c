#include <clib/cybergraphics_protos.h>
#include <pragmas/cybergraphics_pragmas.h>
extern struct Library *CyberGfxBase;
struct List *AllocCModeListTags(Tag tag1, ...)
{
 return(AllocCModeListTagList((struct TagItem *) &tag1));
}
