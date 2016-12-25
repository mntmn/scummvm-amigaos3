#ifndef FS_NODE_AMIGAOS3_H
#define FS_NODE_AMIGAOS3_H

#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>

#include "backends/fs/abstract-fs.h"

class AmigaOS3FilesystemNode : public AbstractFSNode {
protected:
	BPTR _pFileLock;

	Common::String _sDisplayName;
	Common::String _sPath;
	bool _bIsDirectory;
	bool _bIsValid;
	uint32 _nProt;

	virtual AbstractFSList listVolumes() const;
public:
	AmigaOS3FilesystemNode();
	AmigaOS3FilesystemNode(const Common::String &p);
	AmigaOS3FilesystemNode(BPTR pLock, const char *pDisplayName = NULL);
	AmigaOS3FilesystemNode(const AmigaOS3FilesystemNode& node);
	virtual ~AmigaOS3FilesystemNode();

    virtual Common::String getName() const;
    virtual Common::String getPath() const;
	virtual bool isDirectory() const;

	virtual bool exists() const;
	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;
	virtual bool isReadable() const;
	virtual bool isWritable() const;
	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();
	virtual bool create(bool isDirectory);
};

#endif
