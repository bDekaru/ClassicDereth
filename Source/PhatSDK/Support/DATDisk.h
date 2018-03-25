
#pragma once


#ifdef PRE_TOD_DATA_FILES
#define DAT_HEADER_OFFSET 0x12C
#else
#define DAT_HEADER_OFFSET 0x140
#endif

#ifdef PRE_TOD_DATA_FILES

/*
00000000 CLDiskFileInfo_t struc ; (sizeof=0x2C, align=0x4, copyof_1984)
00000000                                         ; XREF: CLDiskHeaderBlock_t/r
00000000 magic_          dd ?
00000004 iBlockSize_     dd ?
00000008 fileSize_       dd ?
0000000C iteration_lm    dd ?
00000010 firstFree_      dd ?
00000014 finalFree_      dd ?
00000018 iFreeBlocks_    dd ?
0000001C btreeRoot_      dd ?
00000020 young_lru_lm    dd ?
00000024 old_lru_lm      dd ?
00000028 use_lru_fm      dd ?
0000002C CLDiskFileInfo_t end
*/

struct DATHeader
{
	DWORD FileType; // 0x00 'TB' !
	DWORD BlockSize; // 0x04 0x400 for PORTAL : 0x100 for CELL
	DWORD FileSize; // 0x08 Should match file size.
	DWORD Iteration; // 0x0C Version iteration.
	DWORD FreeHead; // 0x10
	DWORD FreeTail; // 0x14
	DWORD FreeCount; // 0x18
	DWORD BTree; // 0x1C BTree offset
	DWORD Unknown0; // 0x20
	DWORD Unknown1; // 0x24
	DWORD Unknown2; // 0x28
};
#else

/*
00000000 DiskFileInfo_t  struc ; (sizeof=0x50, align=0x4, copyof_2854)
00000000                                         ; XREF: DiskHeaderBlock_t/r
00000000 magic_          dd ?
00000004 iBlockSize_     dd ?
00000008 fileSize_       dd ?
0000000C data_set_lm     dd ?                    ; enum DATFILE_TYPE
00000010 data_subset_lm  dd ?
00000014 firstFree_      dd ?
00000018 finalFree_      dd ?
0000001C iFreeBlocks_    dd ?
00000020 btreeRoot_      dd ?                    ; XREF: DiskDev::Open_File(PStringBase<ushort> const &,PStringBase<ushort> const &,DiskFileInfo_t *,ulong):loc_676E82/o
00000024 young_lru_lm    dd ?
00000028 old_lru_lm      dd ?
0000002C use_lru_fm      db ?
0000002D                 db ? ; undefined
0000002E                 db ? ; undefined
0000002F                 db ? ; undefined
00000030 master_map_id_m IDClass<_tagDataID,32,0> ?
00000034 eng_pack_vnum   dd ?
00000038 game_pack_vnum  dd ?
0000003C id_vnum         DatIDStamp ?            ; XREF: DiskDev::Open_File(PStringBase<ushort> const &,PStringBase<ushort> const &,DiskFileInfo_t *,ulong)+D4/r
0000003C                                         ; DiskDev::Open_File(PStringBase<ushort> const &,PStringBase<ushort> const &,DiskFileInfo_t *,ulong):loc_676E22/r
00000050 DiskFileInfo_t  ends
*/

#pragma pack(push, 4)
struct DATHeader
{
	DWORD FileType;
	DWORD BlockSize;
	DWORD FileSize;
	DWORD DataSet; // 1 = portal
	DWORD DataSubset;
	DWORD FreeHead;
	DWORD FreeTail;
	DWORD FreeCount;
	DWORD BTree;
	DWORD NewLRU; // 0
	DWORD OldLRU; // 0
	bool bUseLRU; // False
	DWORD MasterMapID;
	DWORD EnginePackVersion;
	DWORD GamePackVersion; // 0
	BYTE VersionMajor[16];
	DWORD VersionMinor;
};
#pragma pack(pop)
#endif

#ifdef PRE_TOD_DATA_FILES
struct BTEntry
{
	DWORD GID_;
	DWORD Offset_;
	DWORD size_;
};
#else
struct BTEntry
{
	DWORD _bf0;
	DWORD GID_;
	DWORD Offset_;
	DWORD size_;
	DWORD date_;
	DWORD iter_; // iteration
};
#endif

struct BTNode
{
	DWORD BlockSpacer;
	DWORD Branches[0x3E];
	DWORD EntryCount;
	BTEntry Entries[0x3D];
};

class BlockLoader;

class BTreeNode
{
public:
	BTreeNode(BlockLoader *pBlockLoader);
	virtual ~BTreeNode();

	BOOL LoadData(DWORD BlockHead);
	void LoadChildren();
	void LoadChildrenRecursive();

	void SetFileCallback(void(*)(void *, DWORD, BTEntry *));
	void SetProgressCallback(void(*)(void *, float));
	void SetCallbackArg(void *);

	BOOL Lookup(DWORD ID, BTEntry *pEntry);

	void FindEntryIDsWithinRange(DWORD Min, DWORD Max, float Progress, float ProgressDelta);

protected:

	DWORD GetBranchCount() const;
	BTreeNode *GetBranch(DWORD index);

	// Using this design, you can't run 2 scans at the same time.
	static void(*m_pfnFileCallback)(void *, DWORD, BTEntry *);
	static void(*m_pfnProgressCallback)(void *, float);
	static void *m_pCallbackArg;

	BlockLoader *m_pBlockLoader;

	BTNode m_TreeData;
	BTreeNode* m_Branches[0x3E];

	BOOL m_bLeaf;
};

class BTree : public BTreeNode
{
public:
	BTree(BlockLoader *pBlockLoader);
	virtual ~BTree();

	BOOL Init();
};

class DiskDev
{
public:
	DiskDev();
	~DiskDev();

	BOOL OpenFile(const char* Path, DATHeader *pHeader);
	void CloseFile();

	BOOL SyncRead(void *pBuffer, DWORD dwLength, DWORD dwPosition);
	BOOL SyncWrite(void *pBuffer, DWORD dwLength, DWORD dwPosition);

private:

	HANDLE m_hFile;
};

class BlockLoader
{
public:
	BlockLoader();
	~BlockLoader();

	BOOL Init(const char *Path, DATHeader *pHeader);

	DWORD GetTreeOrigin();

	BOOL LoadData(DWORD HeadBlock, void *pBuffer, DWORD Length);

private:

	DATHeader *m_pHeader;
	DiskDev m_DiskDev;

};

struct DATEntry
{
	DWORD ID;
	BYTE *Data;
	DWORD Length;
};

class DATDisk
{
public:
	static BOOL OpenDisks(const char *portalPath, const char *cellPath);
	static void CloseDisks();
	static DATDisk *pPortal;
	static DATDisk *pCell;

	DATDisk(const char *Path);
	~DATDisk();

	BOOL Open();
	BOOL GetData(DWORD ID, DATEntry *pEntry);
	BOOL GetDataEx(DWORD BlockHead, void *Data, DWORD Length);
	void FindFileIDsWithinRange(DWORD Min, DWORD Max, void(*FileCallback)(void *, DWORD, BTEntry *), void(*ProgressCallback)(void *, float), void *CallbackArg);

	const DATHeader *GetHeader();

private:
	char *m_FilePath;

	DATHeader m_DATHeader;
	BlockLoader m_BlockLoader;
	BTree m_BTree;
};




