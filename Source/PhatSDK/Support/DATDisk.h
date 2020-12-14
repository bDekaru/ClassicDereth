
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
	uint32_t FileType; // 0x00 'TB' !
	uint32_t BlockSize; // 0x04 0x400 for PORTAL : 0x100 for CELL
	uint32_t FileSize; // 0x08 Should match file size.
	uint32_t Iteration; // 0x0C Version iteration.
	uint32_t FreeHead; // 0x10
	uint32_t FreeTail; // 0x14
	uint32_t FreeCount; // 0x18
	uint32_t BTree; // 0x1C BTree offset
	uint32_t Unknown0; // 0x20
	uint32_t Unknown1; // 0x24
	uint32_t Unknown2; // 0x28
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
	uint32_t FileType;
	uint32_t BlockSize;
	uint32_t FileSize;
	uint32_t DataSet; // 1 = portal
	uint32_t DataSubset;
	uint32_t FreeHead;
	uint32_t FreeTail;
	uint32_t FreeCount;
	uint32_t BTree;
	uint32_t NewLRU; // 0
	uint32_t OldLRU; // 0
	bool bUseLRU; // False
	uint32_t MasterMapID;
	uint32_t EnginePackVersion;
	uint32_t GamePackVersion; // 0
	BYTE VersionMajor[16];
	uint32_t VersionMinor;
};
#pragma pack(pop)
#endif

#ifdef PRE_TOD_DATA_FILES
struct BTEntry
{
	uint32_t GID_;
	uint32_t Offset_;
	uint32_t size_;
};
#else
struct BTEntry
{
	uint32_t _bf0;
	uint32_t GID_;
	uint32_t Offset_;
	uint32_t size_;
	uint32_t date_;
	uint32_t iter_; // iteration
};
#endif

struct BTNode
{
	uint32_t BlockSpacer;
	uint32_t Branches[0x3E];
	uint32_t EntryCount;
	BTEntry Entries[0x3D];
};

class BlockLoader;

class BTreeNode
{
public:
	BTreeNode(BlockLoader *pBlockLoader);
	virtual ~BTreeNode();

	BOOL LoadData(uint32_t BlockHead);
	void LoadChildren();
	void LoadChildrenRecursive();

	void SetFileCallback(void(*)(void *, uint32_t, BTEntry *));
	void SetProgressCallback(void(*)(void *, float));
	void SetCallbackArg(void *);

	BOOL Lookup(uint32_t ID, BTEntry *pEntry);

	void FindEntryIDsWithinRange(uint32_t Min, uint32_t Max, float Progress, float ProgressDelta);

protected:

	uint32_t GetBranchCount() const;
	BTreeNode *GetBranch(uint32_t index);

	// Using this design, you can't run 2 scans at the same time.
	static void(*m_pfnFileCallback)(void *, uint32_t, BTEntry *);
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

	BOOL SyncRead(void *pBuffer, uint32_t dwLength, uint32_t dwPosition);
	BOOL SyncWrite(void *pBuffer, uint32_t dwLength, uint32_t dwPosition);

private:
	using mapped_file_t = mio::basic_mmap<mio::access_mode::read, uint8_t>;

	//HANDLE m_hFile;
	mapped_file_t m_file;
	size_t m_length;
};

class BlockLoader
{
public:
	BlockLoader();
	~BlockLoader();

	BOOL Init(const char *Path, DATHeader *pHeader);

	uint32_t GetTreeOrigin();

	BOOL LoadData(uint32_t HeadBlock, void *pBuffer, uint32_t Length);

private:

	DATHeader *m_pHeader;
	DiskDev m_DiskDev;

};

struct DATEntry
{
	uint32_t ID;
	BYTE *Data;
	uint32_t Length;
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
	BOOL GetData(uint32_t ID, DATEntry *pEntry);
	BOOL GetDataEx(uint32_t BlockHead, void *Data, uint32_t Length);
	void FindFileIDsWithinRange(uint32_t Min, uint32_t Max, void(*FileCallback)(void *, uint32_t, BTEntry *), void(*ProgressCallback)(void *, float), void *CallbackArg);

	const DATHeader *GetHeader();

private:
	char *m_FilePath;

	DATHeader m_DATHeader;
	BlockLoader m_BlockLoader;
	BTree m_BTree;
};




