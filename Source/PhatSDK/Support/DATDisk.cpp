
#include <StdAfx.h>
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
	m_FilePath = strdup(Path);

	// ZeroMemory(&m_DATHeader, sizeof(DATHeader));
	memset(&m_DATHeader, 0, sizeof(DATHeader));
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

void DATDisk::FindFileIDsWithinRange(uint32_t Min, uint32_t Max, void(*FileCallback)(void *, uint32_t, BTEntry *), void(*ProgressCallback)(void *, float), void *CallbackArg)
{
	m_BTree.SetFileCallback(FileCallback);
	m_BTree.SetProgressCallback(ProgressCallback);
	m_BTree.SetCallbackArg(CallbackArg);

	m_BTree.FindEntryIDsWithinRange(Min, Max, 0, 100.0);
}

BOOL DATDisk::GetData(uint32_t ID, DATEntry *pEntry)
{
	BTEntry FileInfo;

	// Look up the file within the BTree.
	if (!m_BTree.Lookup(ID, &FileInfo))
		return FALSE;

	// Attempt to allocate a file buffer. (Extra uint32_t for block loading)
	BYTE *Buffer = new BYTE[sizeof(uint32_t) + FileInfo.size_];

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

BOOL DATDisk::GetDataEx(uint32_t BlockHead, void *Data, uint32_t Length)
{
	if (!m_BlockLoader.LoadData(BlockHead, Data, Length))
		return FALSE;

	return TRUE;
}

void(*BTreeNode::m_pfnFileCallback)(void *, uint32_t, BTEntry *);
void(*BTreeNode::m_pfnProgressCallback)(void *, float);
void *BTreeNode::m_pCallbackArg = NULL;

BTreeNode::BTreeNode(BlockLoader *pBlockLoader)
{
	m_pBlockLoader = pBlockLoader;

	for (uint32_t i = 0; i < 0x3E; i++)
		m_Branches[i] = NULL;
}

BTreeNode::~BTreeNode()
{
	for (uint32_t i = 0; i < 0x3E; i++)
	{
		if (m_Branches[i])
			delete m_Branches[i];
	}
}

BOOL BTreeNode::LoadData(uint32_t BlockHead)
{
	if (!BlockHead)
		return FALSE;

	if (!m_pBlockLoader->LoadData(BlockHead, &m_TreeData, sizeof(BTNode) - sizeof(uint32_t)))
		return FALSE;

	m_bLeaf = (!m_TreeData.Branches[0] ? TRUE : FALSE);

	return TRUE;
}

uint32_t BTreeNode::GetBranchCount() const
{
	if (m_bLeaf)
		return 0;

	return(m_TreeData.EntryCount + 1);
}

void BTreeNode::LoadChildren(void)
{
	for (uint32_t i = 0; i < GetBranchCount(); i++)
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
	for (uint32_t i = 0; i < GetBranchCount(); i++)
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

void BTreeNode::SetFileCallback(void(*pfnFileCallback)(void *, uint32_t, BTEntry *))
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

BTreeNode* BTreeNode::GetBranch(uint32_t index)
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

BOOL BTreeNode::Lookup(uint32_t ID, BTEntry *pEntry)
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

void BTreeNode::FindEntryIDsWithinRange(uint32_t Min, uint32_t Max, float Progress, float ProgressDelta)
{
	uint32_t NumBranches = GetBranchCount();
	float BranchDelta = (NumBranches ? ProgressDelta / NumBranches : 0);
	float ProgressStart = Progress;

	unsigned int i;

	for (i = 0; i < m_TreeData.EntryCount; i++)
	{
		uint32_t ID = m_TreeData.Entries[i].GID_;

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

uint32_t BlockLoader::GetTreeOrigin()
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

BOOL BlockLoader::LoadData(uint32_t HeadBlock, void *pBuffer, uint32_t Length)
{
	BYTE* pbBuffer = (BYTE *)pBuffer;
	uint32_t dwLength = Length;

	uint32_t dwBlockSize = m_pHeader->BlockSize;
	uint32_t dwDataPerBlock = dwBlockSize - sizeof(uint32_t);

	BOOL bLoadOK = TRUE;

	if (!dwLength)
		return TRUE;

	uint32_t dwBlock = HeadBlock;

	while (dwBlock)
	{
		if (!bLoadOK)
			return FALSE;

		uint32_t dwOldValue = *((uint32_t *)pbBuffer);

		if (dwDataPerBlock > dwLength)
		{
			dwBlockSize = dwLength + sizeof(uint32_t);
			dwDataPerBlock = dwLength;
		}

		if (m_DiskDev.SyncRead(pbBuffer, dwBlockSize, dwBlock))
		{
			dwBlock = *((uint32_t *)pbBuffer);
			*((uint32_t *)pbBuffer) = dwOldValue;
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
	//m_hFile = INVALID_HANDLE_VALUE;
}

DiskDev::~DiskDev()
{
	CloseFile();
}

BOOL DiskDev::OpenFile(const char *Path, DATHeader *pHeader)
{
	CloseFile();

	std::error_code err;
	m_file.map(std::string(Path), err);

	if (err)
		return FALSE;

	m_length = m_file.length();

	if (pHeader)
		return SyncRead(pHeader, sizeof(DATHeader), DAT_HEADER_OFFSET);

	return TRUE;

	//m_hFile = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	//if (m_hFile == INVALID_HANDLE_VALUE)
	//	return FALSE;

	//if (pHeader)
	//{
	//	if (!SyncRead(pHeader, sizeof(DATHeader), DAT_HEADER_OFFSET))
	//		return FALSE;
	//}

	//return TRUE;
}

void DiskDev::CloseFile()
{
	if (m_file.is_open())
		m_file.unmap();

	//if (m_hFile != INVALID_HANDLE_VALUE)
	//{
	//	CloseHandle(m_hFile);
	//	m_hFile = INVALID_HANDLE_VALUE;
	//}
}

BOOL DiskDev::SyncRead(void *pBuffer, uint32_t dwLength, uint32_t dwPosition)
{
	mapped_file_t::const_pointer ptr = m_file.cbegin();
	if (!ptr)
		return FALSE;

	if (dwPosition + dwLength >= m_length)
		return FALSE;

	memcpy(pBuffer, ptr + dwPosition, dwLength);
	
	return TRUE;
	
	//if (INVALID_SET_FILE_POINTER == SetFilePointer(m_hFile, dwPosition, 0, FILE_BEGIN))
	//	return FALSE;

	//uint32_t dwBytesRead;

	//if (!ReadFile(m_hFile, pBuffer, dwLength, &dwBytesRead, FALSE))
	//	return FALSE;

	//if (dwBytesRead != dwLength)
	//	return FALSE;

	//return TRUE;
}

BOOL DiskDev::SyncWrite(void *pBuffer, uint32_t dwLength, uint32_t dwPosition)
{
	return FALSE;
	//if (INVALID_SET_FILE_POINTER == SetFilePointer(m_hFile, dwPosition, 0, FILE_BEGIN))
	//	return FALSE;

	//uint32_t dwBytesWritten;

	//if (!WriteFile(m_hFile, pBuffer, dwLength, &dwBytesWritten, FALSE))
	//	return FALSE;

	//if (dwBytesWritten != dwLength)
	//	return FALSE;

	//return TRUE;
}







