/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-1-0-0/backends/fs/amigaos4/amigaos4-fs.cpp $
 * $Id: amigaos4-fs.cpp 36171 2009-02-01 09:49:24Z wjpalenstijn $
 */




#include "backends/fs/amigaos3/amigaos3-fs-node.h"
#include "backends/fs/stdiostream.h"

#include "common/debug.h"

#ifndef PATH_MAX
 #define PATH_MAX 1024
#endif

/****************************************************************************/
/* Handy macros for checking what kind of object a ExAllData's
   ed_Type describes;  ExAll() */

#define EAD_IS_FILE(ead)    ((ead)->ed_Type <  0)

#define EAD_IS_DRAWER(ead)  ((ead)->ed_Type >= 0 && \
                             (ead)->ed_Type != ST_SOFTLINK)

#define EAD_IS_LINK(ead)    ((ead)->ed_Type == ST_SOFTLINK || \
                             (ead)->ed_Type == ST_LINKDIR || \
                             (ead)->ed_Type == ST_LINKFILE)

#define EAD_IS_SOFTLINK(ead) ((ead)->ed_Type == ST_SOFTLINK)

#define EAD_IS_LINKDIR(ead)  ((ead)->ed_Type == ST_LINKDIR)


/****************************************************************************/
/* Handy macros for checking what kind of object a FileInfoBlock
 * describes; Examine() / ExNext().
 * Special versions of these  macros for ExAll() are available
 * in the include file;  dos/exall.h
 */

#define FIB_IS_FILE(fib)      ((fib)->fib_DirEntryType <  0)

#define FIB_IS_DRAWER(fib)    ((fib)->fib_DirEntryType >= 0 && \
                               (fib)->fib_DirEntryType != ST_SOFTLINK)

#define FIB_IS_LINK(fib)      ((fib)->fib_DirEntryType == ST_SOFTLINK || \
                               (fib)->fib_DirEntryType == ST_LINKDIR || \
                               (fib)->fib_DirEntryType == ST_LINKFILE)

#define FIB_IS_SOFTLINK(fib)  ((fib)->fib_DirEntryType == ST_SOFTLINK)

#define FIB_IS_LINKDIR(fib)   ((fib)->fib_DirEntryType == ST_LINKDIR)


// 16K ExAllData buffer.
#define FILE_DATA_BUFFER  16384

// 64K ExAllData buffer.
//#define FILE_DATA_BUFFER    65536









/**
 * Returns the last component of a given path.
 *
 * @param str Common::String containing the path.
 * @return Pointer to the first char of the last component inside str.
 */
static const char *lastPathComponent(const Common::String &str) {
#ifndef NDEBUG
    debug(8, "lastPathComponent(%s)", str.c_str());
#endif

	int offset = str.size();

	if (offset <= 0) {
		return 0;
	}

	const char *p = str.c_str();

	while (offset > 0 && (p[offset-1] == '/' || p[offset-1] == ':'))
		offset--;

	while (offset > 0 && (p[offset-1] != '/' && p[offset-1] != ':'))
		offset--;

	return p + offset;
}

/**
 * Obtain the FileInfoBlock protection value for this FSNode,
 * as defined in the <proto/dos.h> header.
 *
 * @return -1 if there were errors, 0 or a positive integer otherwise.
 */
static int getFibProtection(Common::String path) {
#ifndef NDEBUG
    debug(8, "getFibProtection(%s)", path.c_str());
#endif

	int fibProt = -1;
	struct FileInfoBlock *fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, TAG_END);
	if (!fib) {
		return fibProt;
	}

	BPTR pLock = Lock((STRPTR)path.c_str(), SHARED_LOCK);
	if (pLock) {
		if (Examine(pLock, fib) != DOSFALSE) {
			fibProt = fib->fib_Protection;
		}
		UnLock(pLock);
	}

	FreeDosObject(DOS_FIB, fib);

	return fibProt;
}

static void CopyStringBSTRToC(BPTR bptr, char *destination) {
#ifndef NDEBUG
    debug(8, "CopyStringBSTRToC(bptr, %s)", destination);
#endif

    const char *source = (const char*)BADDR(bptr);
    size_t num = *source++;

    strncpy(destination, source, num);
    destination[num] = 0;
}


AbstractFSList AmigaOS3FilesystemNode::listVolumes() const {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::listVolumes()");
#endif

    AbstractFSList myList;
    char buffer[MAXPATHLEN];

    struct DosList *dosList = LockDosList(LDF_READ | LDF_VOLUMES | LDF_DEVICES);
    if (!dosList) {
    	return myList;
    }

    // List the volumes.
    struct DosList *volumeList = dosList;

    volumeList = NextDosEntry(volumeList, LDF_VOLUMES);

    while (volumeList) {
        if (volumeList->dol_Type == DLT_VOLUME && volumeList->dol_Name && volumeList->dol_Task) {

            // Find device name
            char *devName = new char[MAXPATHLEN];

            struct DosList *deviceList = volumeList;

            deviceList = NextDosEntry(deviceList, LDF_DEVICES);

            while (deviceList) {
                if (deviceList->dol_Task == volumeList->dol_Task) {
                    CopyStringBSTRToC(deviceList->dol_Name, devName);
                    break;
                }

                deviceList = NextDosEntry(deviceList, LDF_DEVICES);
            }


            // Find the volume name
            CopyStringBSTRToC(volumeList->dol_Name, buffer);

            // Volume name + '\0'
			char *volName = new char[strlen(buffer) + 1];

            strcpy(volName, buffer);

			strcat(buffer, ":");

            BPTR volumeLock = Lock((STRPTR)buffer, SHARED_LOCK);
            if (volumeLock) {
                char *displayName = new char[strlen(volName) + strlen(devName) + 3];
                sprintf(displayName, "%s (%s)", volName, devName);

                AmigaOS3FilesystemNode *entry = new AmigaOS3FilesystemNode(volumeLock, displayName);
                delete[] displayName;

                if (entry) {
                    myList.push_back(entry);
                }

                UnLock(volumeLock);
            }

            delete[] devName;
            delete[] volName;

            volumeList = NextDosEntry(volumeList, LDF_VOLUMES);
        }
    }

    UnLockDosList(LDF_READ | LDF_VOLUMES | LDF_DEVICES);


    return myList;
}





AmigaOS3FilesystemNode::AmigaOS3FilesystemNode() {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::AmigaOS3FilesystemNode()");
#endif

	_sDisplayName = "Available Volumes";
	_bIsValid = true;
	_bIsDirectory = true;
	_sPath = "";
	_pFileLock = 0;
}

AmigaOS3FilesystemNode::AmigaOS3FilesystemNode(const Common::String &p) {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::AmigaOS3FilesystemNode(%s)", p.c_str());
#endif

	int offset = p.size();

	if (offset <= 0) {
		return;
	}

	_sPath = p;
	_sDisplayName = ::lastPathComponent(_sPath);
	_pFileLock = 0;
	_bIsDirectory = false;

	struct FileInfoBlock *fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, TAG_END);
    if (!fib) {
		return;
	}

	// Check whether the node exists and if it is a directory
	BPTR pLock = Lock((STRPTR)_sPath.c_str(), SHARED_LOCK);
	if (pLock) {
		if (Examine(pLock, fib) != DOSFALSE) {
			if (FIB_IS_DRAWER(fib)) {
				_bIsDirectory = true;
				_pFileLock = DupLock(pLock);
				_bIsValid = (_pFileLock != 0);

				// Add a trailing slash if it is needed
				const char c = _sPath.lastChar();
				if (c != '/' && c != ':')
					_sPath += '/';
			}
			else {
				_bIsValid = true;
			}
		}

		UnLock(pLock);
	}

	FreeDosObject(DOS_FIB, fib);
}

AmigaOS3FilesystemNode::AmigaOS3FilesystemNode(BPTR pLock, const char *pDisplayName) {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::AmigaOS3FilesystemNode(pLock, %s)", pDisplayName);

    assert (pLock);
#endif


	int bufSize = MAXPATHLEN;
	_pFileLock = 0;

	while (true) {
		char *n = new char[bufSize];
		if (NameFromLock(pLock, (STRPTR)n, bufSize) != DOSFALSE) {
			_sPath = n;
			_sDisplayName = pDisplayName ? pDisplayName : FilePart((STRPTR)n);
			delete[] n;
			break;
		}

		if (IoErr() != ERROR_LINE_TOO_LONG) {
			_bIsValid = false;
			delete[] n;
			return;
		}

		bufSize *= 2;
		delete[] n;
	}

	_bIsValid =	false;
	_bIsDirectory = false;

	struct FileInfoBlock *fib = (struct	FileInfoBlock *)AllocDosObject(DOS_FIB, TAG_END);
	if (!fib) {
        return;
	}

	if (Examine(pLock, fib) != DOSFALSE) {
		if (FIB_IS_DRAWER(fib)) {
			_bIsDirectory = true;
			_pFileLock = DupLock(pLock);
			_bIsValid = _pFileLock != 0;

			const char c = _sPath.lastChar();
			if (c != '/' && c != ':')
				_sPath += '/';
		}
		else {
			_bIsValid = true;
		}
	}

	FreeDosObject(DOS_FIB, fib);
}

// We need the custom copy constructor because of DupLock()
AmigaOS3FilesystemNode::AmigaOS3FilesystemNode(const AmigaOS3FilesystemNode& node) {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::AmigaOS3FilesystemNode(AmigaOS3FilesystemNode)");
#endif

	_sDisplayName = node._sDisplayName;
	_bIsValid = node._bIsValid;
	_bIsDirectory = node._bIsDirectory;
	_sPath = node._sPath;
	_pFileLock = DupLock(node._pFileLock);
}

AmigaOS3FilesystemNode::~AmigaOS3FilesystemNode() {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::~AmigaOS3FilesystemNode()");
#endif

	if (_pFileLock) {
		UnLock(_pFileLock);
    }
}

bool AmigaOS3FilesystemNode::exists() const {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::exists(): %s", _sPath.c_str());
#endif

	if(_sPath.empty()) {
        return false;
    }

	bool nodeExists = false;

	BPTR pLock = Lock((STRPTR)_sPath.c_str(), SHARED_LOCK);
	if (pLock) {
        nodeExists = true;
		UnLock(pLock);
    }

    debug(8, "-> %d\n", nodeExists);
	return nodeExists;
}

AbstractFSNode *AmigaOS3FilesystemNode::getChild(const Common::String &n) const {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::getChild()");
#endif

	if (!_bIsDirectory) {
		return 0;
	}

	Common::String newPath(_sPath);

	if (_sPath.lastChar() != '/')
		newPath += '/';

	newPath += n;

	return new AmigaOS3FilesystemNode(newPath);
}

bool AmigaOS3FilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::getChildren()");
#endif

	//TODO: honor the hidden flag

	if (!_bIsValid) {
		return false; // Empty list
	}

	if (!_bIsDirectory) {
		return false; // Empty list
	}

	if (_pFileLock == 0) {
		myList = listVolumes();
		return true;
	}

	struct ExAllControl *eac = (struct ExAllControl *)AllocDosObject(DOS_EXALLCONTROL, TAG_END);
	if (eac) {
		struct ExAllData *data = (struct ExAllData *)AllocVec(FILE_DATA_BUFFER, MEMF_ANY);
		if (data) {
			BOOL bExMore;
			eac->eac_LastKey = 0;
			do {
				// Examine directory
				bExMore = ExAll(_pFileLock, data, FILE_DATA_BUFFER, ED_TYPE, eac);

				LONG error = IoErr();
				if (!bExMore && error != ERROR_NO_MORE_ENTRIES)
					break; // Abnormal failure

				if (eac->eac_Entries ==	0)
					continue; // Normal failure, no entries

				struct ExAllData *ead = data;
				do {
					if ((mode == Common::FSNode::kListAll) ||
						(EAD_IS_DRAWER(ead) && (mode == Common::FSNode::kListDirectoriesOnly)) ||
						(EAD_IS_FILE(ead) && (mode == Common::FSNode::kListFilesOnly))) {
						Common::String full_path = _sPath;
						full_path += (char*)ead->ed_Name;

						BPTR lock = Lock((STRPTR)full_path.c_str(), SHARED_LOCK);
						if (lock) {
							AmigaOS3FilesystemNode *entry = new AmigaOS3FilesystemNode(lock, (char *)ead->ed_Name);
							if (entry) {
							    myList.push_back(entry);
							}

							UnLock(lock);
						}
					}
					ead = ead->ed_Next;
				} while (ead);
			} while (bExMore);

			FreeVec(data);
		}

		FreeDosObject(DOS_EXALLCONTROL, eac);
	}

	return true;
}


AbstractFSNode *AmigaOS3FilesystemNode::getParent() const {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::getParent()");
#endif

	if (!_bIsDirectory) {
		return 0;
	}

	if (_pFileLock == 0) {
		return new AmigaOS3FilesystemNode(*this);
	}

	AmigaOS3FilesystemNode *node;

	BPTR parentDir = ParentDir( _pFileLock );
	if (parentDir) {
		node = new AmigaOS3FilesystemNode(parentDir);
		UnLock(parentDir);
	} else {
		node = new AmigaOS3FilesystemNode();
    }

	return node;
}

bool AmigaOS3FilesystemNode::isReadable() const {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::isReadable()");
#endif

    bool readable = false;
	int fibProt = getFibProtection(_sPath);

	if (fibProt >= 0) {
		/* The fib_Protection flag is low-active or inverted, thus the negation.
		 *
		 * For more information, consult the compiler/include/dos/dos.h
		 * file from the AROS source (http://aros.sourceforge.net/).
		 */
		readable = !(fibProt & FIBF_READ);
	}

	return readable;
}

bool AmigaOS3FilesystemNode::isWritable() const {
#ifndef NDEBUG
    debug(8, "AmigaOS3FilesystemNode::isWritable()");
#endif

	bool writable = false;
	int fibProt = getFibProtection(_sPath);

	if (fibProt >= 0) {
		/* The fib_Protection flag is low-active or inverted, thus the negation.
		 *
		 * For more information, consult the compiler/include/dos/dos.h
		 * file from the AROS source (http://aros.sourceforge.net/).
		 */
		writable = !(fibProt & FIBF_WRITE);
	}

	return writable;
}






Common::SeekableReadStream *AmigaOS3FilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath().c_str(), false);
}

Common::WriteStream *AmigaOS3FilesystemNode::createWriteStream() {
	return StdioStream::makeFromPath(getPath().c_str(), true);
}


Common::String AmigaOS3FilesystemNode::getName() const {
	return _sDisplayName;
}
Common::String AmigaOS3FilesystemNode::getPath() const {
	return _sPath;
}
bool AmigaOS3FilesystemNode::isDirectory() const {
	return _bIsDirectory;
}

bool AmigaOS3FilesystemNode::create(bool isDirectory) {
	error("Not supported");
	return false;
}
                