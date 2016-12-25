#ifndef FS_FACTORY_AMIGAOS3_H
#define FS_FACTORY_AMIGAOS3_H

#include "backends/fs/fs-factory.h"

class AmigaOS3FilesystemFactory : public FilesystemFactory {
 	virtual AbstractFSNode *makeRootFileNode() const;
 	virtual AbstractFSNode *makeCurrentDirectoryFileNode() const;
	virtual AbstractFSNode *makeFileNodePath(const Common::String &path) const;
};

#endif
