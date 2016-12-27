#include <clib/cybergraphics_protos.h>
#include <pragmas/cybergraphics_pragmas.h>
extern struct Library *CyberGfxBase;
ULONG CModeRequestTags(APTR moderequest, Tag tag1, ...)
{
 return(CModeRequestTagList(moderequest, (struct TagItem *) &tag1));
}
