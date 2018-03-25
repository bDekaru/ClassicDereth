
#include "StdAfx.h"
#include "DATDisk.h"

DATDisk *DATDisk::pPortal = NULL;
DATDisk *DATDisk::pCell = NULL;

BOOL DATDisk::OpenDisks(const char *portalPath, const char *cellPath)
{
	pPortal = new DATDisk(portalPath);

	if (!pPortal->Open())
		return FALSE;

	pCell = new DATDisk(cellPath);

	if (!pCell->Open())
		return FALSE;

	return TRUE;
}

void DATDisk::CloseDisks()
{
	if (pPortal)
	{
		delete pPortal;
		pPortal = NULL;
	}

	if (pCell)
	{
		delete pCell;
		pCell = NULL;
	}
}

DATDisk::DATDisk(const char *Path) : m_BTree(&m_BlockLoader)
{
	m_FilePath = _strdup(Path);

	ZeroMemory(&m_DATHeader, sizeof(DATHeader));
}

DATDisk::~DATDisk()
{
	if (m_FilePath)
		free(m_FilePath);
}

BOOL DATDisk::Open()
{
	if (!m_BlockLoader.Init(m_FilePath, &m_DATHeader))
		return FALSE;

	if (!m_BTree.Init())
		return FALSE;

	return TRUE;
}

const DATHeader *DATDisk::GetHeader()
{
	return &m_DATHeader;
}

void DATDisk::FindFileIDsWithinRange(DWORD Min, DWORD Max, void(*FileCallback)(void *, DWORD, BTEntry *), void(*ProgressCallback)(void *, float), void *CallbackArg)
{
	m_BTree.SetFileCallback(FileCallback);
	m_BTree.SetProgressCallback(ProgressCallback);
	m_BTree.SetCallbackArg(CallbackArg);

	m_BTree.FindEntryIDsWithinRange(Min, Max, 0, 100.0);
}

BOOL DATDisk::GetData(DWORD ID, DATEntry *pEntry)
{
	BTEntry FileInfo;

	// Look up the file within the BTree.
	if (!m_BTree.Lookup(ID, &FileInfo))
		return FALSE;

	// Attempt to allocate a file buffer. (Extra DWORD for block loading)
	BYTE *Buffer = new BYTE[sizeof(DWORD) + FileInfo.size_];

	if (!Buffer)
		return FALSE;

	if (!m_BlockLoader.LoadData(FileInfo.Offset_, Buffer, FileInfo.size_))
	{
		delete[] Buffer;
		return FALSE;
	}

	if (pEntry)
	{
		pEntry->ID = ID;
		pEntry->Data = Buffer;
		pEntry->Length = FileInfo.size_;
	}
	else
		delete[] Buffer;

	return TRUE;
}

BOOL DATDisk::GetDataEx(DWORD BlockHead, void *Data, DWORD Length)
{
	if (!m_BlockLoader.LoadData(BlockHead, Data, Length))
		return FALSE;

	return TRUE;
}

void(*BTreeNode::m_pfnFileCallback)(void *, DWORD, BTEntry *);
void(*BTreeNode::m_pfnProgressCallback)(void *, float);
void *BTreeNode::m_pCallbackArg = NULL;

BTreeNode::BTreeNode(BlockLoader *pBlockLoader)
{
	m_pBlockLoader = pBlockLoader;

	for (DWORD i = 0; i < 0x3E; i++)
		m_Branches[i] = NULL;
}

BTreeNode::~BTreeNode()
{
	for (DWORD i = 0; i < 0x3E; i++)
	{
		if (m_Branches[i])
			delete m_Branches[i];
	}
}

BOOL BTreeNode::LoadData(DWORD BlockHead)
{
	if (!BlockHead)
		return FALSE;

	if (!m_pBlockLoader->LoadData(BlockHead, &m_TreeData, sizeof(BTNode) - sizeof(DWORD)))
		return FALSE;

	m_bLeaf = (!m_TreeData.Branches[0] ? TRUE : FALSE);

	return TRUE;
}

DWORD BTreeNode::GetBranchCount() const
{
	if (m_bLeaf)
		return 0;

	return(m_TreeData.EntryCount + 1);
}

void BTreeNode::LoadChildren(void)
{
	for (DWORD i = 0; i < GetBranchCount(); i++)
	{
		if (m_Branches[i])
			continue;

		m_Branches[i] = new BTreeNode(m_pBlockLoader);

		if (!m_Branches[i])
			continue;

		if (!m_Branches[i]->LoadData(m_TreeData.Branches[i]))
		{
			delete m_Branches[i];
			m_Branches[i] = NULL;
		}
	}
}

void BTreeNode::LoadChildrenRecursive(void)
{
	for (DWORD i = 0; i < GetBranchCount(); i++)
	{
		if (m_Branches[i])
			continue;

		m_Branches[i] = new BTreeNode(m_pBlockLoader);

		if (!m_Branches[i])
			continue;

		if (m_Branches[i]->LoadData(m_TreeData.Branches[i]))
			m_Branches[i]->LoadChildrenRecursive();
		else
		{
			delete m_Branches[i];
			m_Branches[i] = NULL;
		}
	}
}

void BTreeNode::SetFileCallback(void(*pfnFileCallback)(void *, DWORD, BTEntry *))
{
	m_pfnFileCallback = pfnFileCallback;
}

void BTreeNode::SetProgressCallback(void(*pfnProgressCallback)(void *, float))
{
	m_pfnProgressCallback = pfnProgressCallback;
}

void BTreeNode::SetCallbackArg(void *CallbackArg)
{
	m_pCallbackArg = CallbackArg;
}

BTreeNode* BTreeNode::GetBranch(DWORD index)
{
	if (!m_Branches[index])
	{
		if (m_Branches[index] = new BTreeNode(m_pBlockLoader))
		{
			if (!m_Branches[index]->LoadData(m_TreeData.Branches[index]))
			{
				delete m_Branches[index];
				m_Branches[index] = NULL;
			}
		}
	}

	return m_Branches[index];
}

BOOL BTreeNode::Lookup(DWORD ID, BTEntry *pEntry)
{
	unsigned int i;

	for (i = 0; i < m_TreeData.EntryCount; i++)
	{
		if (ID < m_TreeData.Entries[i].GID_)
		{
			BTreeNode *Branch;

			if (!m_bLeaf && (Branch = GetBranch(i)))
				return Branch->Lookup(ID, pEntry);
			else
				return FALSE;
		}

		if (ID == m_TreeData.Entries[i].GID_)
		{
			if (pEntry)
				*pEntry = m_TreeData.Entries[i];

			return TRUE;
		}
	}

	BTreeNode *LastBranch;

	if (!m_bLeaf && (LastBranch = GetBranch(i)))
		return LastBranch->Lookup(ID, pEntry);
	else
		return FALSE;
}

void BTreeNode::FindEntryIDsWithinRange(DWORD Min, DWORD Max, float Progress, float ProgressDelta)
{
	DWORD NumBranches = GetBranchCount();
	float BranchDelta = (NumBranches ? ProgressDelta / NumBranches : 0);
	float ProgressStart = Progress;

	unsigned int i;

	for (i = 0; i < m_TreeData.EntryCount; i++)
	{
		DWORD ID = m_TreeData.Entries[i].GID_;

		if (ID > Max)
		{
			BTreeNode *Branch;

			if (!m_bLeaf && (Branch = GetBranch(i)))
				Branch->FindEntryIDsWithinRange(Min, Max, Progress, BranchDelta);

			return;
		}

		if (ID < Min)
			continue;

		BTreeNode *Branch;

		if (!m_bLeaf && (Branch = GetBranch(i)))
			Branch->FindEntryIDsWithinRange(Min, Max, Progress, BranchDelta);

		if (m_pfnFileCallback)
			m_pfnFileCallback(m_pCallbackArg, ID, &m_TreeData.Entries[i]);

		if (!m_bLeaf)
		{
			Progress += BranchDelta;

			if (m_pfnProgressCallback)
				m_pfnProgressCallback(m_pCallbackArg, Progress);
		}
	}

	if (!m_bLeaf)
	{
		BTreeNode *LastBranch;

		if (LastBranch = GetBranch(i))
			LastBranch->FindEntryIDsWithinRange(Min, Max, Progress, BranchDelta);
	}

	Progress = ProgressStart + ProgressDelta;

	if (m_pfnProgressCallback)
		m_pfnProgressCallback(m_pCallbackArg, Progress);
}

BTree::BTree(BlockLoader *pBlockLoader) : BTreeNode(pBlockLoader)
{
}

BTree::~BTree()
{
}

BOOL BTree::Init()
{
	if (!LoadData(m_pBlockLoader->GetTreeOrigin()))
		return FALSE;

	// Preload immediate children. ?
	// LoadChildren();
	// Preload ALL children. ? (very slow)

#ifndef QUICKSTART
	//LoadChildrenRecursive();
#endif

	return TRUE;
}

BlockLoader::BlockLoader()
{
	m_pHeader = NULL;
}

BlockLoader::~BlockLoader()
{
}

DWORD BlockLoader::GetTreeOrigin()
{
	if (!m_pHeader)
		return 0;

	return m_pHeader->BTree;
}

BOOL BlockLoader::Init(const char *Path, DATHeader *pHeader)
{
	m_pHeader = pHeader;

	return m_DiskDev.OpenFile(Path, m_pHeader);
}

BOOL BlockLoader::LoadData(DWORD HeadBlock, void *pBuffer, DWORD Length)
{
	BYTE* pbBuffer = (BYTE *)pBuffer;
	DWORD dwLength = Length;

	DWORD dwBlockSize = m_pHeader->BlockSize;
	DWORD dwDataPerBlock = dwBlockSize - sizeof(DWORD);

	BOOL bLoadOK = TRUE;

	if (!dwLength)
		return TRUE;

	DWORD dwBlock = HeadBlock;

	while (dwBlock)
	{
		if (!bLoadOK)
			return FALSE;

		DWORD dwOldValue = *((DWORD *)pbBuffer);

		if (dwDataPerBlock > dwLength)
		{
			dwBlockSize = dwLength + sizeof(DWORD);
			dwDataPerBlock = dwLength;
		}

		if (m_DiskDev.SyncRead(pbBuffer, dwBlockSize, dwBlock))
		{
			dwBlock = *((DWORD *)pbBuffer);
			*((DWORD *)pbBuffer) = dwOldValue;
			dwLength -= dwDataPerBlock;
			pbBuffer += dwDataPerBlock;

			if (dwBlock & 0x80000000)
			{
				dwBlock &= ~0x80000000;
				bLoadOK = FALSE;
			}
		}
		else
			bLoadOK = FALSE;

		if (!dwLength)
			break;
	}

	if (dwLength)
		bLoadOK = FALSE;

	return bLoadOK;
}

DiskDev::DiskDev()
{
	m_hFile = INVALID_HANDLE_VALUE;
}

DiskDev::~DiskDev()
{
	CloseFile();
}

BOOL DiskDev::OpenFile(const char *Path, DATHeader *pHeader)
{
	CloseFile();

	m_hFile = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	if (pHeader)
	{
		if (!SyncRead(pHeader, sizeof(DATHeader), DAT_HEADER_OFFSET))
			return FALSE;
	}

	return TRUE;
}

void DiskDev::CloseFile()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

BOOL DiskDev::SyncRead(void *pBuffer, DWORD dwLength, DWORD dwPosition)
{
	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_hFile, dwPosition, 0, FILE_BEGIN))
		return FALSE;

	DWORD dwBytesRead;

	if (!ReadFile(m_hFile, pBuffer, dwLength, &dwBytesRead, FALSE))
		return FALSE;

	if (dwBytesRead != dwLength)
		return FALSE;

	return TRUE;
}

BOOL DiskDev::SyncWrite(void *pBuffer, DWORD dwLength, DWORD dwPosition)
{
	if (INVALID_SET_FILE_POINTER == SetFilePointer(m_hFile, dwPosition, 0, FILE_BEGIN))
		return FALSE;

	DWORD dwBytesWritten;

	if (!WriteFile(m_hFile, pBuffer, dwLength, &dwBytesWritten, FALSE))
		return FALSE;

	if (dwBytesWritten != dwLength)
		return FALSE;

	return TRUE;
}







