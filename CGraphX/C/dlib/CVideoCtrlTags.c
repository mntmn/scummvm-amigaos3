#include <clib/cybergraphics_protos.h>
#include <pragmas/cybergraphics_pragmas.h>
extern struct Library *CyberGfxBase;
void CVideoCtrlTags(struct ViewPort *vp, Tag tag1, ...)
{
 return(CVideoCtrlTagList(vp, (struct TagItem *) &tag1));
}
