#include <clib/cybergraphics_protos.h>
#include <pragmas/cybergraphics_pragmas.h>
extern struct Library *CyberGfxBase;
void DoCDrawMethodTags(struct Hook *h, struct RastPort *rp, Tag tag1, ...)
{
 return(DoCDrawMethodTagList(h, rp, (struct TagItem *) &tag1));
}
