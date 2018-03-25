
#include "StdAfx.h"

#if PHATSDK_RENDER_AVAILABLE
#include "Render.h"
#endif

#include "Palette.h"
#include "Texture.h"

#ifdef PRE_TOD

DWORD                        ImgTexManager::num_chunks;
DArray<ImgTexChunk *>        ImgTexManager::chunk_array(256, 256);

void(*ImgTex::textureloss_callback)(DWORD dwLoss) = NULL; //static_cast< void (*)(DWORD) >(NULL);
LongNIValHash<ImgTex *>* ImgTex::temp_buffer_table[MAX_IMAGE_TYPES];
LongNIValHash<ImgTex *>    ImgTex::texture_table; // keyed
LongNIValHash<ImgTex *>    ImgTex::custom_texture_table; // misc
LongNIValHash<ImgTex *>    ImgTex::tiled_image_table; //combined

long    ImgTex::min_tex_size = 8;
long    ImgTex::image_use_type;
BOOL    ImgTex::bSquareTexturesOnly = FALSE;

long    ImgTex::RowHeights[6] = { 8, 16, 32, 64, 128, 256 };

BOOL ImgTexManager::FreeTexture(ImgTexChunkInfo *pChunkInfo)
{
	ImgTexChunk *pChunk = pChunkInfo->m_pChunk;

	if (!pChunk)
		return FALSE;

	BYTE **pLink = (BYTE **)pChunkInfo->m_pPixelData;

	*pLink = pChunk->m_pPixelData;
	pChunk->m_pPixelData = (BYTE *)pLink;

	if (!(--pChunk->m_iUnknown))
	{
		for (DWORD i = 0; i < num_chunks; i++)
		{
			if (pChunk == chunk_array.array_data[i])
			{
				chunk_array.array_data[i] = chunk_array.array_data[--num_chunks];
				break;
			}
		}

		delete pChunk;
	}

	pChunkInfo->m_pPixelData = NULL;
	pChunkInfo->m_pChunk = NULL;

	return TRUE;
}

BOOL ImgTexManager::AllocateTexture(long Width, long Height, ImgTexChunkInfo *pChunkInfo)
{
	long WidthPower = ImgTex::GetPow2(Width);
	long HeightPower = ImgTex::GetPow2(Height);

	for (DWORD i = 0; i < num_chunks; i++)
	{
		ImgTexChunk *pChunk = chunk_array.array_data[i];

		if (pChunk->m_iWidthPow == WidthPower &&
			pChunk->m_iHeightPow == HeightPower &&
			pChunk->m_pPixelData)
		{
			pChunkInfo->m_pChunk = pChunk;
			pChunkInfo->m_pPixelData = pChunk->m_pPixelData;

			pChunkInfo->calc_chunk_offset();

			pChunk->m_pPixelData = *((BYTE **)pChunk->m_pPixelData);
			pChunk->m_iUnknown++;
			return TRUE;
		}
	}

	ImgTexChunk *pChunk = new ImgTexChunk;
	pChunk->SetImageSize(WidthPower, HeightPower);

	BOOL ReturnValue;

	if (pChunk->m_iWidthPow == WidthPower &&
		pChunk->m_iHeightPow == HeightPower &&
		pChunk->m_pPixelData)
	{
		pChunkInfo->m_pChunk = pChunk;
		pChunkInfo->m_pPixelData = pChunk->m_pPixelData;
		pChunkInfo->calc_chunk_offset();

		pChunk->m_pPixelData = *((BYTE **)pChunk->m_pPixelData);
		pChunk->m_iUnknown++;

		ReturnValue = TRUE;
	}
	else
		ReturnValue = FALSE;

	if (num_chunks >= chunk_array.alloc_size)
		chunk_array.grow(num_chunks + 256);

	chunk_array.array_data[num_chunks++] = pChunk;

	return ReturnValue;
}

ImgTexChunk::ImgTexChunk()
{
	m_iWidthPow = 0;
	m_iHeightPow = 0;
	m_pPixelData = NULL;
	m_iUnknown = 0;
}

void ImgTexChunk::SetImageSize(long Width, long Height)
{

	m_iWidthPow = ImgTex::GetPow2(Width);
	m_iHeightPow = ImgTex::GetPow2(Height);

	for (long Y = 0; Y < (256 / m_iHeightPow); Y++)
	{
		for (long X = 1; (X - 1) < (256 / m_iWidthPow); X++)
		{
			long Val = ((X % (256 / m_iWidthPow)) ? X : 0);
			long HP;

			if (X % (256 / m_iWidthPow))
				HP = Y;
			else
				HP = Y + 1;

			BYTE **PixPtr = (BYTE **)(((BYTE *)m_IntPixelMap) +
				((Y * m_iHeightPow) << 8) + ((X - 1) * m_iWidthPow));

			if (HP < (256 / m_iHeightPow))
			{
				*PixPtr = ((BYTE *)m_IntPixelMap) + ((HP * m_iHeightPow) << 8) + (Val * m_iWidthPow);
			}
			else
				*PixPtr = NULL;
		}
	}

	m_pPixelData = (BYTE *)&m_IntPixelMap;
	m_iUnknown = 0;
}

ImgTexChunkInfo::ImgTexChunkInfo()
{
	m_pPixelData = NULL;
	m_pChunk = NULL;
	m_f08 = 0;
	m_f0C = 0;
}

ImgTexChunkInfo::~ImgTexChunkInfo()
{
}

void ImgTexChunkInfo::calc_chunk_offset()
{
	DWORD Offset = (DWORD)m_pPixelData - (DWORD)m_pChunk;

	m_f08 = ((float)(long)(Offset & 0xFF)) + 0.002f;
	m_f0C = ((float)(long)(Offset >> 8)) + 0.002f;
}

ImgTex::ImgTex()
{
	m_lWidth = 0;
	m_lHeight = 0;
	m_lPitch = 0;
	m_pPixels = NULL;
	m_pPalette = NULL;
	m_bInCache = TRUE;

#if PHATSDK_RENDER_AVAILABLE
	m_pD3DTexture9 = NULL;
#endif

#ifdef DEPRECATED_CODE
	m_pDDSurf = NULL;
	m_pSurfaceChunk = NULL;
#endif
}

ImgTex::~ImgTex()
{
	Destroy();
}

void ImgTex::Destroy()
{
	if (m_pPalette)
	{
		ObjCaches::Palettes->Release(m_pPalette->GetID());
		m_pPalette = NULL;
	}

	if (m_pPixels)
	{
		// m_dw4C is a chunk related subclass?
		if (!ImgTexManager::FreeTexture(&m_ChunkInfo))
			delete[] m_pPixels;

		m_pPixels = NULL;
	}

	m_lWidth = 0;
	m_lHeight = 0;
	m_lPitch = 0;

	if (m_pD3DTexture9) {
		m_pD3DTexture9->Release();
		m_pD3DTexture9 = NULL;
	}

	// Deprecated.. Release DirectDraw Surface
	// Deprecated.. Release D3D Chunk Manager..
}

long ImgTex::GetPow2(long Value)
{
	for (DWORD i = 0; i < 6; i++)
	{
		if (Value <= RowHeights[i])
			return RowHeights[i];
	}

	return 0;
}

void ImgTex::SetTextureLossCallback(void(*pfnCallback)(DWORD))
{
	textureloss_callback = pfnCallback;
}

BOOL ImgTex::DoChunkification()
{
#ifdef DEPRECATED_CODE
	if (Device::Does3DFXSuck() && bDoChunkification)
		return TRUE;
#endif

	return FALSE;
}

DBObj* ImgTex::Allocator()
{
	return((DBObj *)new ImgTex());
}

void ImgTex::Destroyer(DBObj* pTexture)
{
	delete ((ImgTex *)pTexture);
}

ImgTex *ImgTex::Get(DWORD ID)
{
	return (ImgTex *)ObjCaches::Textures->Get(ID);
}

void ImgTex::releaseTexture(ImgTex *pTexture)
{
	if (!pTexture)
		return;

	if (pTexture->m_bInCache)
	{
		ObjCaches::Textures->Release(pTexture->GetID());
	}
	else if (pTexture->GetID())
	{
		if (!pTexture->Unlink())
		{
			if (texture_table.remove(pTexture->GetID(), &pTexture))
			{
				// Found, delete it.
				delete pTexture;
			}
		}
	}
	else
	{
		if (custom_texture_table.remove((DWORD)pTexture, &pTexture))
		{
			// Found, delete it.
			delete pTexture;
		}
	}
}

ImgTex *ImgTex::makeCustomTexture(DWORD custom_texture_ID)
{
	ImgTex *pTexture = new ImgTex;

	if (pTexture)
	{
		pTexture->SetID(0);
		pTexture->m_lLinks = 0;
		pTexture->m_bInCache = FALSE;

		if (custom_texture_ID)
		{
			pTexture->SetID(custom_texture_ID);
			pTexture->Link();

			texture_table.add(pTexture, custom_texture_ID);
		}
		else
		{
			custom_texture_table.add(pTexture, (DWORD)pTexture);
		}
	}

	return pTexture;
}

ImgTex *ImgTex::makeTempTexture()
{
	ImgTex *pTexture = new ImgTex;

	if (pTexture)
	{
		pTexture->SetID(0);
		pTexture->m_lLinks = 0;
		pTexture->m_bInCache = FALSE;

		custom_texture_table.add(pTexture, (DWORD)pTexture);
	}

	return pTexture;
}

ImgTex *ImgTex::getCustomTexture(DWORD custom_texture_ID)
{
	ImgTex *pEntry = NULL;

	if (texture_table.lookup(custom_texture_ID, &pEntry))
		pEntry->Link();

	return pEntry;
}

ImgTex *ImgTex::AllocateTempBuffer(long Width, long Height, long Type)
{
	ImgTex *pTempBuffer = makeTempTexture();

	if (!pTempBuffer)
		return NULL;

	if (!pTempBuffer->InitOther(Width, Height, Type, 0x20, TRUE))
	{
		// Failed init, get rid of it.
		releaseTexture(pTempBuffer);
		return NULL;
	}

	return pTempBuffer;
}

BOOL ImgTex::InitTemporaryBuffer()
{
	for (DWORD i = 0; i < MAX_IMAGE_TYPES; i++)
		temp_buffer_table[i] = NULL;

	return TRUE;
}

void ImgTex::ReleaseTemporaryBuffer()
{
	for (DWORD i = 0; i < MAX_IMAGE_TYPES; i++)
	{
		if (temp_buffer_table[i])
		{
			LongNIValHashIter<ImgTex *> Iter(temp_buffer_table[i]);

			while (!Iter.EndReached())
			{
				LongNIValHashData<ImgTex *> *pData = Iter.GetCurrent();

				ImgTex *pImgTex = (pData ? pData->m_Data : NULL);

				ImgTex::releaseTexture(pImgTex);

				Iter.DeleteCurrent();
			}
		}
	}
}

void ImgTex::CleanupTemporaryBuffer()
{
	ReleaseTemporaryBuffer();

	for (DWORD i = 0; i < MAX_IMAGE_TYPES; i++)
	{
		if (temp_buffer_table[i])
		{
			delete temp_buffer_table[i];
			temp_buffer_table[i] = NULL;
		}
	}
}

void ImgTex::releaseAllSurfaces()
{
}

ImgTex *ImgTex::GetTempBuffer(long Width, long Height, long Type)
{
	// Create a hash table for this image type, if one doesn't already exist.
	LongNIValHash<ImgTex *>* pTempBufferTable = temp_buffer_table[Type];

	if (!pTempBufferTable)
	{
		temp_buffer_table[Type] = pTempBufferTable = new LongNIValHash<ImgTex *>;

		if (!pTempBufferTable)
			return NULL;
	}

	ImgTex *pTempImage = NULL;
	DWORD TempKey = (Width << 16) | (Height & 0xFFFF);

	// Does an entry already exist?
	if (pTempBufferTable->lookup(TempKey, &pTempImage))
		return pTempImage;

	// Create one.
	pTempImage = AllocateTempBuffer(Width, Height, Type);

	if (!pTempImage)
		return FALSE; // Failed! Don't insert!

	pTempBufferTable->add(pTempImage, TempKey);

	return pTempImage;
}

LPVOID ImgTex::GetData()
{
	if (!m_pD3DTexture9)
		return NULL;

	D3DLOCKED_RECT LockedRect;

	if (FAILED(m_pD3DTexture9->LockRect(0, &LockedRect, NULL, 0)))
		return NULL;

	m_lPitch = LockedRect.Pitch;
	return LockedRect.pBits;

	/*
	DDSURFACEDESC2 SurfaceDesc;

	ZeroMemory(&SurfaceDesc, sizeof(SurfaceDesc));
	SurfaceDesc.dwSize = sizeof(SurfaceDesc);

	// Lock the surface!
	if (FAILED(m_pDDSurf->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL)))
	return NULL;

	m_lPitch = SurfaceDesc.lPitch;
	return SurfaceDesc.lpSurface;
	*/
}

BOOL ImgTex::Combine(ImgTex* pImgTex, Palette* pPalette, BOOL bUnknown, long Type)
{
	BOOL bCached;

	if (bUnknown)
		Type = 3;

	ImgTex *pDummy;
	bCached = tiled_image_table.lookup(pImgTex->GetID(), &pDummy);

	// Init me
	if (!InitOther(pImgTex->m_lWidth, pImgTex->m_lHeight, Type, 0x1000, bCached))
		return FALSE;

	// Create a temporary image buffer.
	ImgTex *pTempImg = GetTempBuffer(pImgTex->m_lWidth, pImgTex->m_lHeight, Type);

	if (!pTempImg)
		return FALSE;

	// Use the temporary image data.
	LPVOID lpData = pTempImg->GetData();

	if (!lpData)
		return FALSE;

	CopyIntoData(lpData, pTempImg->m_lWidth, pTempImg->m_lHeight, pTempImg->m_lPitch, pTempImg->m_lType, pImgTex, pPalette, bUnknown);

	// Unlock the Surface
	// pTempImg->UnlockDDSurf();
	pTempImg->UnlockTexture9();

	if (!Load(pTempImg))
		return FALSE;

	return TRUE;
}

BOOL ImgTex::CopyIntoData(LPVOID lpData, long Width, long Height, long Pitch, DWORD arg_10, ImgTex *pSourceImg, Palette *pPalette, BOOL bUnknown)
{
	// Source/Target dimensions must match!
	if (pSourceImg->m_lWidth != Width || pSourceImg->m_lHeight != Height)
		return FALSE;

	LPVOID lpNextData = lpData;

	for (long Y = 0; Y < pSourceImg->m_lHeight; Y++)
	{
		LPVOID lpSourceData = (LPVOID)(((BYTE *)pSourceImg->m_pPixels) + (pSourceImg->m_lPitch * Y));

		lpData = lpNextData;

		// Next row will be designated by the data pitch.
		lpNextData = (LPVOID)(((BYTE *)lpData) + Pitch);

		for (long X = 0; X < pSourceImg->m_lWidth; X++)
		{
			BYTE bColorIndex = ((BYTE *)lpSourceData)[X];
			WORD wColorARGB;

			if (bUnknown)
			{
				if (bColorIndex) {
					WORD wPaletteColor = pPalette->get_color(bColorIndex);

					// RGB 565 to ARGB 1555 ?
					wColorARGB = /*Alpha*/0x8000 | /*Red/Green*/((wPaletteColor >> 1) & 0xFFE0) | /*Blue*/(wPaletteColor & 0x1F);
				}
				else
					wColorARGB = 0;
			}
			else
				wColorARGB = pPalette->get_color(bColorIndex);

			*((WORD *)lpData) = wColorARGB;
			lpData = (LPVOID)(((WORD *)lpData) + 1);
		}
	}

	return TRUE;
}

BOOL ImgTex::InitOther(long Width, long Height, long Type, DWORD Flags, BOOL InCache)
{
	if (m_pD3DTexture9)
		return FALSE;

	if (!g_pD3DDevice)
		return FALSE;

	D3DFORMAT Format;

	switch (Type)
	{
	case 3:
		// ARGB 1555 Format.
		Format = D3DFMT_A1R5G5B5;
		break;
	case 4:
		// ARGB 4444 Format.
		Format = D3DFMT_A4R4G4B4;
		break;
	case 7:
		// RGB 565 Format.
		Format = D3DFMT_R5G6B5;
		break;
	default:
		return FALSE;
	}

	BOOL bSuccess = FALSE;

	// If ARG_C & 0x1000 this should be in video memory.
	if (SUCCEEDED(g_pD3DDevice->CreateTexture(
		Width, Height, 1, 0, Format, (Flags & 0x1000) ? D3DPOOL_DEFAULT : D3DPOOL_SYSTEMMEM,
		&m_pD3DTexture9, NULL)))
	{
		bSuccess = TRUE;
	}

	m_lType = Type;
	m_lWidth = Width;
	m_lHeight = Height;

	if (!bSuccess)
	{
		// Failed. Destroy me.
		Destroy();
	}

	return bSuccess;
}

BOOL ImgTex::InitIndex8(long Width, long Height, long Type)
{
	if (!ImgTexManager::AllocateTexture(Width, Height, &m_ChunkInfo))
		return FALSE;

	m_pPixels = m_ChunkInfo.m_pPixelData;
	m_lWidth = Width;
	m_lHeight = Height;
	m_lPitch = 0x100;
	m_lType = Type;

	return TRUE;
}

BOOL ImgTex::InitCSI(long Width, long Height, long Type)
{
	UNFINISHED();
	return FALSE;
}

BOOL ImgTex::InitAlpha(long Width, long Height, long Type)
{
	UNFINISHED();
	return FALSE;
}

BOOL ImgTex::Init(long Width, long Height, long Type, BOOL InCache)
{
	switch (Type)
	{
	case 2:    return InitIndex8(Width, Height, 2);
	case 10: return InitCSI(Width, Height, 10);
	case 11: return InitAlpha(Width, Height, 11);
	default: return InitOther(Width, Height, Type, 0x1000, InCache);
	}
}

BOOL ImgTex::CopyIntoTexture(long Width, long Height, long Pitch, BYTE *Data)
{
	ImgTex *pTempTex = GetTempBuffer(m_lWidth, m_lHeight, m_lType);

	if (!pTempTex)
		return FALSE;

	LPVOID lpTempData = pTempTex->GetData();

	if (!lpTempData)
		return FALSE;

	switch (m_lType)
	{
	case 3:
	case 4:
	case 7:

		CopySrcIntoDst(
			Width, Height, Pitch, Data, sizeof(WORD),
			pTempTex->m_lWidth, pTempTex->m_lHeight,
			pTempTex->m_lPitch, lpTempData, sizeof(WORD));
		break;
	}

	pTempTex->UnlockTexture9();

	if (!Load(pTempTex))
		return FALSE;

	return TRUE;
}

BOOL ImgTex::CopySrcIntoDst(
	long sWidth, long sHeight, long sPitch, LPVOID sData, int sPSize,
	long dWidth, long dHeight, long dPitch, LPVOID dData, int dPSize)
{

	if (sPSize != dPSize)
		return FALSE;

	if ((sWidth < dWidth) || (sHeight < dHeight))
		return FALSE;

	if (sWidth % dWidth)
		return FALSE;

	if (sHeight % dHeight)
		return FALSE;

	long WidthScale = sWidth / dWidth;
	long HeightScale = sHeight / dHeight;

	BYTE *dNextLine = (BYTE *)dData;
	BYTE *sNextLine = (BYTE *)sData;
	BYTE *dPixels;
	BYTE *sPixels;

	long Y;
	long dP;

	for (Y = 0; Y < (sHeight - HeightScale); Y += HeightScale)
	{
		dPixels = dNextLine;
		dNextLine = dPixels + dPitch;

		sPixels = sNextLine;

		dP = 0;

		for (long X = 0; X < (sWidth - WidthScale); X += WidthScale)
		{
			for (long sP = 0; sP < sPSize;)
				dPixels[dP++] = sPixels[sP++];

			sPixels += WidthScale * sPSize;
		}

		sPixels = sNextLine + ((sWidth - 1) * sPSize);

		for (long P = 0; P < sPSize; P++)
		{
			for (long sP = 0; sP < sPSize;)
				dPixels[dP++] = sPixels[sP++];

			sPixels += WidthScale * sPSize;
		}

		sNextLine += HeightScale * sPitch;
	}

	dPixels = (BYTE *)dData + (Y * dPitch);
	dP = 0;
	sPixels = (BYTE *)sData + ((sHeight - 1) * sPitch);
	sNextLine = sPixels;

	for (long X = 0; X < (sWidth - WidthScale); X += WidthScale)
	{
		for (long sP = 0; sP < sPSize;)
			dPixels[dP++] = sPixels[sP++];

		sPixels += WidthScale * sPSize;
	}

	dPixels = &dPixels[dP];
	sPixels = sNextLine + ((sWidth - 1) * sPSize);

	for (long sP = 0; sP < sPSize;)
		*(dPixels++) = sPixels[sP++];

	return TRUE;
}

BOOL ImgTex::UnPack(BYTE **ppData, ULONG iSize)
{
	Destroy();

	long ImgType, ImgWidth, ImgHeight, SrcWidth, SrcHeight;

	UNPACK(DWORD, id);
	UNPACK(long, ImgType);

	if (!ImgType)
	{
		// CFTOD, not handled yet?
		// FIX ME
		return FALSE;
	}

	UNPACK(long, ImgWidth);
	UNPACK(long, ImgHeight);

	SrcWidth = ImgWidth;
	SrcHeight = ImgHeight;

	const DWORD TextureScales[4] = { 0, 1, 2, 4 };

	switch (ImgType)
	{
	case 2:
		{
			switch (image_use_type)
			{
			case 1:
				// TextureScales[ ImgTex::fClipmapTextureScale ];
				break;
			case 2:
				// TextureScales[ ImgTex::fLandTextureScale ];
				break;
			default:
				// TextureScales[ ImgTex::fIndexedTextureScale ];
				break;
			}

			// Adjust width/height

			break;
		}
	case 3:
	case 4:
	case 7:
		// TextureScales[ ImgTex::fRGBATextureScale ];

		// Adjust width/height
		break;
	}

	if (ImgWidth < min_tex_size)
		ImgWidth = min_tex_size;

	if (ImgHeight < min_tex_size)
		ImgHeight = min_tex_size;

	if (bSquareTexturesOnly)
	{
		if (ImgWidth < ImgHeight)
			ImgHeight = ImgWidth;
		else
			ImgWidth = ImgHeight;
	}

	ImgTex *dummy;
	BOOL bInCache = tiled_image_table.lookup(GetID(), &dummy);

	if (!Init(ImgWidth, ImgHeight, ImgType, bInCache))
	{
		m_bInCache = TRUE;
		return FALSE;
	}

	BOOL bSuccess = TRUE;

	switch (ImgType)
	{
	case 2:
		{
			if (m_pPixels)
			{
				CopySrcIntoDst(
					SrcWidth, SrcHeight, SrcWidth, *ppData, sizeof(BYTE),
					m_lWidth, m_lHeight, m_lPitch, m_pPixels, sizeof(BYTE));
			}

			*ppData = *ppData + (SrcWidth * SrcHeight);

			DWORD PaletteID;
			UNPACK(DWORD, PaletteID);

			m_pPalette = Palette::Get(PaletteID);

			if (!m_pPalette)
				bSuccess = FALSE;

			break;
		}

	case 3:
	case 4:
	case 7:
		CopyIntoTexture(SrcWidth, SrcHeight, SrcWidth * sizeof(WORD), *ppData);

		*ppData = *ppData + (SrcWidth * SrcHeight * sizeof(WORD));
		break;

	case 10:
		memcpy(m_pPixels, *ppData, m_lWidth * m_lHeight * sizeof(RGBTRIPLE));

		*ppData = *ppData + (SrcWidth * SrcHeight * sizeof(RGBTRIPLE));
		break;

	case 11:

		memcpy(m_pPixels, *ppData, m_lWidth * m_lHeight * sizeof(BYTE));

		*ppData = *ppData + (SrcWidth * SrcHeight * sizeof(BYTE));
		break;

	default:
		bSuccess = FALSE;
		break;
	}

#ifdef PRE_TOD
	PACK_ALIGN();
#endif
	m_bInCache = TRUE;

	return bSuccess;
}


// Replacements for deprecated
void ImgTex::UnlockTexture9()
{
	if (m_pD3DTexture9)
	{
		m_pD3DTexture9->UnlockRect(0);
		m_lPitch = 0;
	}
}

BOOL ImgTex::Load(ImgTex* pSource)
{
	if (m_lType != pSource->m_lType)
		return FALSE;
	if (m_lWidth != pSource->m_lWidth || m_lHeight != pSource->m_lHeight)
		return FALSE;

	if (!m_pD3DTexture9 || !pSource->m_pD3DTexture9)
		return FALSE;


	GetData();
	g_pD3DDevice->UpdateTexture(pSource->m_pD3DTexture9, m_pD3DTexture9);
	UnlockTexture9();

	// m_pD3DTexture9->Release();
	// m_pD3DTexture9 = pSource->m_pD3DTexture9;
	// m_pD3DTexture9->AddRef();

	// pSource->m_pD3DTexture9 = NULL;

	return TRUE;
}

#ifdef DEPRECATED_CODE

// These are deprecated
BOOL ImgTex::QueryTextureInterface()
{
	__asm int 3;
}

void ImgTex::UnlockDDSurf()
{
	__asm int 3;
}

#endif

ImgColor::ImgColor()
{
	/*
	m_dw18 = 0;
	m_dw1C = 0;
	m_dw20 = 0;
	m_dw24 = 0;

	m_lWidth = 0;
	m_lHeight = 0;
	m_lPitch = 0;
	m_pPixels = NULL;
	m_pPalette = NULL;
	m_bInCache = TRUE;
	m_pD3DTexture9 = NULL;

	#ifdef DEPRECATED_CODE
	m_pDDSurf = NULL;
	m_pSurfaceChunk = NULL;
	#endif
	*/
}

ImgColor::~ImgColor()
{
	/*
	Destroy();
	*/
}

DBObj* ImgColor::Allocator()
{
	return((DBObj *)new ImgColor());
}

void ImgColor::Destroyer(DBObj* pImgColor)
{
	delete ((ImgTex *)pImgColor);
}

ImgColor *ImgColor::Get(DWORD ID)
{
	return (ImgColor *)ObjCaches::ImgColors->Get(ID);
}

BOOL ImgColor::UnPack(BYTE **ppData, ULONG iSize)
{
	UNPACK(DWORD, id);
	UNPACK(DWORD, m_Width);
	UNPACK(DWORD, m_Height);

	if ((((m_Width * m_Height) + 4) * 3) > iSize)
	{
		__asm int 3;
		return FALSE;
	}

	// Init(m_Width, m_Height);

	return TRUE;
}

/*
BOOL ImgColor::Init(DWORD width, DWORD height)
{
	UNFINISHED();
	return FALSE;
}
*/

#else

// 64-bit warnings that are perfectly correct but.. this is the way AC works and it PROBABLY wont break anything if memory stays within a relatively small range
#pragma warning(disable: 4311)
#pragma warning(disable: 4302)

// this code is NOT REVERSE ENGINEERED.. i made it up for throne of destiny

DWORD ImgTexManager::num_chunks;
DArray<ImgTexChunk *> ImgTexManager::chunk_array(256, 256);

void(*ImgTex::textureloss_callback)(DWORD dwLoss) = NULL; //static_cast< void (*)(DWORD) >(NULL);
LongNIValHash<ImgTex *>* ImgTex::temp_buffer_table[MAX_IMAGE_TYPES];
LongNIValHash<ImgTex *> ImgTex::texture_table; // keyed
LongNIValHash<ImgTex *> ImgTex::custom_texture_table; // misc
LongNIValHash<ImgTex *> ImgTex::tiled_image_table; //combined

long ImgTex::min_tex_size = 8;
long ImgTex::image_use_type;
BOOL ImgTex::bSquareTexturesOnly = FALSE;

long ImgTex::RowHeights[6] = { 8, 16, 32, 64, 128, 256 };

BOOL ImgTexManager::FreeTexture(ImgTexChunkInfo *pChunkInfo)
{
	ImgTexChunk *pChunk = pChunkInfo->m_pChunk;

	if (!pChunk)
		return FALSE;

	BYTE **pLink = (BYTE **)pChunkInfo->m_pPixelData;

	*pLink = pChunk->m_pPixelData;
	pChunk->m_pPixelData = (BYTE *)pLink;

	if (!(--pChunk->m_iUnknown))
	{
		for (DWORD i = 0; i < num_chunks; i++)
		{
			if (pChunk == chunk_array.array_data[i])
			{
				chunk_array.array_data[i] = chunk_array.array_data[--num_chunks];
				break;
			}
		}

		delete pChunk;
	}

	pChunkInfo->m_pPixelData = NULL;
	pChunkInfo->m_pChunk = NULL;

	return TRUE;
}

BOOL ImgTexManager::AllocateTexture(long Width, long Height, ImgTexChunkInfo *pChunkInfo)
{
	long WidthPower = ImgTex::GetPow2(Width);
	long HeightPower = ImgTex::GetPow2(Height);

	for (DWORD i = 0; i < num_chunks; i++)
	{
		ImgTexChunk *pChunk = chunk_array.array_data[i];

		if (pChunk->m_iWidthPow == WidthPower &&
			pChunk->m_iHeightPow == HeightPower &&
			pChunk->m_pPixelData)
		{
			pChunkInfo->m_pChunk = pChunk;
			pChunkInfo->m_pPixelData = pChunk->m_pPixelData;

			pChunkInfo->calc_chunk_offset();

			pChunk->m_pPixelData = *((BYTE **)pChunk->m_pPixelData);
			pChunk->m_iUnknown++;
			return TRUE;
		}
	}

	ImgTexChunk *pChunk = new ImgTexChunk;
	pChunk->SetImageSize(WidthPower, HeightPower);

	BOOL ReturnValue;

	if (pChunk->m_iWidthPow == WidthPower &&
		pChunk->m_iHeightPow == HeightPower &&
		pChunk->m_pPixelData)
	{
		pChunkInfo->m_pChunk = pChunk;
		pChunkInfo->m_pPixelData = pChunk->m_pPixelData;
		pChunkInfo->calc_chunk_offset();

		pChunk->m_pPixelData = *((BYTE **)pChunk->m_pPixelData);
		pChunk->m_iUnknown++;

		ReturnValue = TRUE;
	}
	else
		ReturnValue = FALSE;

	if (num_chunks >= chunk_array.alloc_size)
		chunk_array.grow(num_chunks + 256);

	chunk_array.array_data[num_chunks++] = pChunk;

	return ReturnValue;
}

ImgTexChunk::ImgTexChunk()
{
	m_iWidthPow = 0;
	m_iHeightPow = 0;
	m_pPixelData = NULL;
	m_iUnknown = 0;
}

void ImgTexChunk::SetImageSize(long Width, long Height)
{

	m_iWidthPow = ImgTex::GetPow2(Width);
	m_iHeightPow = ImgTex::GetPow2(Height);

	for (long Y = 0; Y < (256 / m_iHeightPow); Y++)
	{
		for (long X = 1; (X - 1) < (256 / m_iWidthPow); X++)
		{
			long Val = ((X % (256 / m_iWidthPow)) ? X : 0);
			long HP;

			if (X % (256 / m_iWidthPow))
				HP = Y;
			else
				HP = Y + 1;

			BYTE **PixPtr = (BYTE **)(((BYTE *)m_IntPixelMap) +
				((Y * m_iHeightPow) << 8) + ((X - 1) * m_iWidthPow));

			if (HP < (256 / m_iHeightPow))
			{
				*PixPtr = ((BYTE *)m_IntPixelMap) + ((HP * m_iHeightPow) << 8) + (Val * m_iWidthPow);
			}
			else
				*PixPtr = NULL;
		}
	}

	m_pPixelData = (BYTE *)&m_IntPixelMap;
	m_iUnknown = 0;
}

ImgTexChunkInfo::ImgTexChunkInfo()
{
	m_pPixelData = NULL;
	m_pChunk = NULL;
	m_f08 = 0;
	m_f0C = 0;
}

ImgTexChunkInfo::~ImgTexChunkInfo()
{
}

void ImgTexChunkInfo::calc_chunk_offset()
{
	DWORD Offset = (DWORD)((BYTE *)m_pPixelData - (BYTE *)m_pChunk);

	m_f08 = ((float)(long)(Offset & 0xFF)) + 0.002f;
	m_f0C = ((float)(long)(Offset >> 8)) + 0.002f;
}

ImgTex::ImgTex()
{
	m_lWidth = 0;
	m_lHeight = 0;
	m_lPitch = 0;
	m_pPixels = NULL;
	m_pPalette = NULL;
	m_bInCache = TRUE;

#if PHATSDK_RENDER_AVAILABLE
	m_pD3DTexture9 = NULL;
#endif

#ifdef DEPRECATED_CODE
	m_pDDSurf = NULL;
	m_pSurfaceChunk = NULL;
#endif
}

ImgTex::~ImgTex()
{
	Destroy();
}

void ImgTex::Destroy()
{
	if (m_pPalette)
	{
		ObjCaches::Palettes->Release(m_pPalette->GetID());
		m_pPalette = NULL;
	}

	if (m_pPixels)
	{
		// m_dw4C is a chunk related subclass?
		if (!ImgTexManager::FreeTexture(&m_ChunkInfo))
			delete[] m_pPixels;

		m_pPixels = NULL;
	}

	m_lWidth = 0;
	m_lHeight = 0;
	m_lPitch = 0;

#if PHATSDK_RENDER_AVAILABLE
	if (m_pD3DTexture9) {
		m_pD3DTexture9->Release();
		m_pD3DTexture9 = NULL;
	}
#endif

	// Deprecated.. Release DirectDraw Surface
	// Deprecated.. Release D3D Chunk Manager..
}

long ImgTex::GetPow2(long Value)
{
	for (DWORD i = 0; i < 6; i++)
	{
		if (Value <= RowHeights[i])
			return RowHeights[i];
	}

	return 0;
}

void ImgTex::SetTextureLossCallback(void(*pfnCallback)(DWORD))
{
	textureloss_callback = pfnCallback;
}

BOOL ImgTex::DoChunkification()
{
#ifdef DEPRECATED_CODE
	if (Device::Does3DFXSuck() && bDoChunkification)
		return TRUE;
#endif

	return FALSE;
}

DBObj* ImgTex::Allocator()
{
	return((DBObj *)new ImgTex());
}

void ImgTex::Destroyer(DBObj* pTexture)
{
	delete ((ImgTex *)pTexture);
}

ImgTex *ImgTex::Get(DWORD ID)
{
	return (ImgTex *)ObjCaches::Textures->Get(ID);
}

void ImgTex::releaseTexture(ImgTex *pTexture)
{
	if (!pTexture)
		return;

	if (pTexture->m_bInCache)
	{
		if (ObjCaches::Textures)
			ObjCaches::Textures->Release(pTexture->GetID());
	}
	else if (pTexture->GetID())
	{
		if (!pTexture->Unlink())
		{
			if (texture_table.remove(pTexture->GetID(), &pTexture))
			{
				// Found, delete it.
				delete pTexture;
			}
		}
	}
	else
	{
		// This could wreak havoc in 64-bit, casting pointer to to 32-bit key...
		if (custom_texture_table.remove((DWORD)pTexture, &pTexture))
		{
			// Found, delete it.
			delete pTexture;
		}
	}
}

ImgTex *ImgTex::makeCustomTexture(DWORD custom_texture_ID)
{
	DebugBreak();

	ImgTex *pTexture = new ImgTex;

	if (pTexture)
	{
		pTexture->SetID(0);
		pTexture->m_lLinks = 0;
		pTexture->m_bInCache = FALSE;

		if (custom_texture_ID)
		{
			pTexture->SetID(custom_texture_ID);
			pTexture->Link();

			texture_table.add(pTexture, custom_texture_ID);
		}
		else
		{
			custom_texture_table.add(pTexture, (DWORD)pTexture);
		}
	}

	return pTexture;
}

ImgTex *ImgTex::makeTempTexture()
{
	DebugBreak();

	ImgTex *pTexture = new ImgTex;

	if (pTexture)
	{
		pTexture->SetID(0);
		pTexture->m_lLinks = 0;
		pTexture->m_bInCache = FALSE;

		custom_texture_table.add(pTexture, (DWORD)pTexture);
	}

	return pTexture;
}

ImgTex *ImgTex::getCustomTexture(DWORD custom_texture_ID)
{
	DebugBreak();

	ImgTex *pEntry = NULL;

	if (texture_table.lookup(custom_texture_ID, &pEntry))
		pEntry->Link();

	return pEntry;
}

ImgTex *ImgTex::AllocateTempBuffer(long Width, long Height, long Type)
{
	ImgTex *pTempBuffer = makeTempTexture();

	if (!pTempBuffer)
		return NULL;

	if (!pTempBuffer->InitOther(Width, Height, Type, 0x20, TRUE))
	{
		// Failed init, get rid of it.
		releaseTexture(pTempBuffer);
		return NULL;
	}

	return pTempBuffer;
}

BOOL ImgTex::InitTemporaryBuffer()
{
	for (DWORD i = 0; i < MAX_IMAGE_TYPES; i++)
		temp_buffer_table[i] = NULL;

	return TRUE;
}

void ImgTex::ReleaseTemporaryBuffer()
{
	for (DWORD i = 0; i < MAX_IMAGE_TYPES; i++)
	{
		if (temp_buffer_table[i])
		{
			LongNIValHashIter<ImgTex *> Iter(temp_buffer_table[i]);

			while (!Iter.EndReached())
			{
				LongNIValHashData<ImgTex *> *pData = Iter.GetCurrent();

				ImgTex *pImgTex = (pData ? pData->m_Data : NULL);

				ImgTex::releaseTexture(pImgTex);

				Iter.DeleteCurrent();
			}
		}
	}
}

void ImgTex::CleanupTemporaryBuffer()
{
	ReleaseTemporaryBuffer();

	for (DWORD i = 0; i < MAX_IMAGE_TYPES; i++)
	{
		if (temp_buffer_table[i])
		{
			delete temp_buffer_table[i];
			temp_buffer_table[i] = NULL;
		}
	}
}

void ImgTex::releaseAllSurfaces()
{
}

ImgTex *ImgTex::GetTempBuffer(long Width, long Height, long Type)
{
	// Create a hash table for this image type, if one doesn't already exist.
	LongNIValHash<ImgTex *>* pTempBufferTable = temp_buffer_table[Type];

	if (!pTempBufferTable)
	{
		temp_buffer_table[Type] = pTempBufferTable = new LongNIValHash<ImgTex *>;

		if (!pTempBufferTable)
			return NULL;
	}

	ImgTex *pTempImage = NULL;
	DWORD TempKey = (Width << 16) | (Height & 0xFFFF);

	// Does an entry already exist?
	if (pTempBufferTable->lookup(TempKey, &pTempImage))
		return pTempImage;

	// Create one.
	pTempImage = AllocateTempBuffer(Width, Height, Type);

	if (!pTempImage)
		return FALSE; // Failed! Don't insert!

	pTempBufferTable->add(pTempImage, TempKey);

	return pTempImage;
}

LPVOID ImgTex::GetData()
{
#if PHATSDK_RENDER_AVAILABLE
	if (!m_pD3DTexture9)
		return NULL;

	D3DLOCKED_RECT LockedRect;

	if (FAILED(m_pD3DTexture9->LockRect(0, &LockedRect, NULL, 0)))
		return NULL;

	m_lPitch = LockedRect.Pitch;
	return LockedRect.pBits;

	/*
	DDSURFACEDESC2 SurfaceDesc;

	ZeroMemory(&SurfaceDesc, sizeof(SurfaceDesc));
	SurfaceDesc.dwSize = sizeof(SurfaceDesc);

	// Lock the surface!
	if (FAILED(m_pDDSurf->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL)))
	return NULL;

	m_lPitch = SurfaceDesc.lPitch;
	return SurfaceDesc.lpSurface;
	*/
#else
	return NULL;
#endif
}

BOOL ImgTex::Combine(ImgTex* pImgTex, Palette* pPalette, BOOL bUnknown, long Type)
{
	BOOL bCached;

	if (bUnknown)
		Type = 3;

	ImgTex *pDummy;
	bCached = tiled_image_table.lookup(pImgTex->GetID(), &pDummy);

	// Init me
	if (!InitOther(pImgTex->m_lWidth, pImgTex->m_lHeight, Type, 0x1000, bCached))
		return FALSE;

	// Create a temporary image buffer.
	ImgTex *pTempImg = GetTempBuffer(pImgTex->m_lWidth, pImgTex->m_lHeight, Type);

	if (!pTempImg)
		return FALSE;

	// Use the temporary image data.
	LPVOID lpData = pTempImg->GetData();

	if (!lpData)
		return FALSE;

	CopyIntoData(lpData, pTempImg->m_lWidth, pTempImg->m_lHeight, pTempImg->m_lPitch, pTempImg->m_lType, pImgTex, pPalette, bUnknown);

	// Unlock the Surface
	// pTempImg->UnlockDDSurf();
	pTempImg->UnlockTexture9();

	if (!Load(pTempImg))
		return FALSE;

	return TRUE;
}

BOOL ImgTex::CopyIntoData(LPVOID lpData, long Width, long Height, long Pitch, DWORD arg_10, ImgTex *pSourceImg, Palette *pPalette, BOOL bUnknown)
{
	// Source/Target dimensions must match!
	if (pSourceImg->m_lWidth != Width || pSourceImg->m_lHeight != Height)
		return FALSE;

	LPVOID lpNextData = lpData;

	for (long Y = 0; Y < pSourceImg->m_lHeight; Y++)
	{
		LPVOID lpSourceData = (LPVOID)(((BYTE *)pSourceImg->m_pPixels) + (pSourceImg->m_lPitch * Y));

		lpData = lpNextData;

		// Next row will be designated by the data pitch.
		lpNextData = (LPVOID)(((BYTE *)lpData) + Pitch);

		for (long X = 0; X < pSourceImg->m_lWidth; X++)
		{
			BYTE bColorIndex = ((BYTE *)lpSourceData)[X];
			WORD wColorARGB;

			if (bUnknown)
			{
				if (bColorIndex) {
					WORD wPaletteColor = pPalette->get_color(bColorIndex);

					// RGB 565 to ARGB 1555 ?
					wColorARGB = /*Alpha*/0x8000 | /*Red/Green*/((wPaletteColor >> 1) & 0xFFE0) | /*Blue*/(wPaletteColor & 0x1F);
				}
				else
					wColorARGB = 0;
			}
			else
				wColorARGB = pPalette->get_color(bColorIndex);

			*((WORD *)lpData) = wColorARGB;
			lpData = (LPVOID)(((WORD *)lpData) + 1);
		}
	}

	return TRUE;
}

BOOL ImgTex::InitOther(long Width, long Height, long Type, DWORD Flags, BOOL InCache)
{
#if PHATSDK_RENDER_AVAILABLE
	if (m_pD3DTexture9)
		return FALSE;

	if (!g_pD3DDevice)
		return FALSE;

	D3DFORMAT Format;

	switch (Type)
	{
	case 3:
		// ARGB 1555 Format.
		Format = D3DFMT_A1R5G5B5;
		break;
	case 4:
		// ARGB 4444 Format.
		Format = D3DFMT_A4R4G4B4;
		break;
	case 7:
		// RGB 565 Format.
		Format = D3DFMT_R5G6B5;
		break;
	default:
		return FALSE;
	}

	BOOL bSuccess = FALSE;

	// If ARG_C & 0x1000 this should be in video memory.
	if (SUCCEEDED(g_pD3DDevice->CreateTexture(
		Width, Height, 1, 0, Format, (Flags & 0x1000) ? D3DPOOL_DEFAULT : D3DPOOL_SYSTEMMEM,
		&m_pD3DTexture9, NULL)))
	{
		bSuccess = TRUE;
	}

	m_lType = Type;
	m_lWidth = Width;
	m_lHeight = Height;

	if (!bSuccess)
	{
		// Failed. Destroy me.
		Destroy();
	}

	return bSuccess;
#else
	return FALSE;
#endif
}

BOOL ImgTex::InitIndex8(long Width, long Height, long Type)
{
	if (!ImgTexManager::AllocateTexture(Width, Height, &m_ChunkInfo))
		return FALSE;

	m_pPixels = m_ChunkInfo.m_pPixelData;
	m_lWidth = Width;
	m_lHeight = Height;
	m_lPitch = 0x100;
	m_lType = Type;

	return TRUE;
}

BOOL ImgTex::InitCSI(long Width, long Height, long Type)
{
	UNFINISHED_LEGACY("ImgTex::InitCSI");
	return FALSE;
}

BOOL ImgTex::InitAlpha(long Width, long Height, long Type)
{
	UNFINISHED_LEGACY("ImgTex::InitAlpha");
	return FALSE;
}

BOOL ImgTex::Init(long Width, long Height, long Type, BOOL InCache)
{
	switch (Type)
	{
	case 2: return InitIndex8(Width, Height, 2);
	case 10: return InitCSI(Width, Height, 10);
	case 11: return InitAlpha(Width, Height, 11);
	default: return InitOther(Width, Height, Type, 0x1000, InCache);
	}
}

BOOL ImgTex::CopyIntoTexture(long Width, long Height, long Pitch, BYTE *Data)
{
	ImgTex *pTempTex = GetTempBuffer(m_lWidth, m_lHeight, m_lType);

	if (!pTempTex)
		return FALSE;

	LPVOID lpTempData = pTempTex->GetData();

	if (!lpTempData)
		return FALSE;

	switch (m_lType)
	{
	case 3:
	case 4:
	case 7:

		CopySrcIntoDst(
			Width, Height, Pitch, Data, sizeof(WORD),
			pTempTex->m_lWidth, pTempTex->m_lHeight,
			pTempTex->m_lPitch, lpTempData, sizeof(WORD));
		break;
	}

	pTempTex->UnlockTexture9();

	if (!Load(pTempTex))
		return FALSE;

	return TRUE;
}

BOOL ImgTex::CopySrcIntoDst(
	long sWidth, long sHeight, long sPitch, LPVOID sData, int sPSize,
	long dWidth, long dHeight, long dPitch, LPVOID dData, int dPSize)
{

	if (sPSize != dPSize)
		return FALSE;

	if ((sWidth < dWidth) || (sHeight < dHeight))
		return FALSE;

	if (sWidth % dWidth)
		return FALSE;

	if (sHeight % dHeight)
		return FALSE;

	long WidthScale = sWidth / dWidth;
	long HeightScale = sHeight / dHeight;

	BYTE *dNextLine = (BYTE *)dData;
	BYTE *sNextLine = (BYTE *)sData;
	BYTE *dPixels;
	BYTE *sPixels;

	long Y;
	long dP;

	for (Y = 0; Y < (sHeight - HeightScale); Y += HeightScale)
	{
		dPixels = dNextLine;
		dNextLine = dPixels + dPitch;

		sPixels = sNextLine;

		dP = 0;

		for (long X = 0; X < (sWidth - WidthScale); X += WidthScale)
		{
			for (long sP = 0; sP < sPSize;)
				dPixels[dP++] = sPixels[sP++];

			sPixels += WidthScale * sPSize;
		}

		sPixels = sNextLine + ((sWidth - 1) * sPSize);

		for (long P = 0; P < sPSize; P++)
		{
			for (long sP = 0; sP < sPSize;)
				dPixels[dP++] = sPixels[sP++];

			sPixels += WidthScale * sPSize;
		}

		sNextLine += HeightScale * sPitch;
	}

	dPixels = (BYTE *)dData + (Y * dPitch);
	dP = 0;
	sPixels = (BYTE *)sData + ((sHeight - 1) * sPitch);
	sNextLine = sPixels;

	for (long X = 0; X < (sWidth - WidthScale); X += WidthScale)
	{
		for (long sP = 0; sP < sPSize;)
			dPixels[dP++] = sPixels[sP++];

		sPixels += WidthScale * sPSize;
	}

	dPixels = &dPixels[dP];
	sPixels = sNextLine + ((sWidth - 1) * sPSize);

	for (long sP = 0; sP < sPSize;)
		*(dPixels++) = sPixels[sP++];

	return TRUE;
}

BOOL ImgTex::UnPack(BYTE **ppData, ULONG iSize)
{
	Destroy();

	UNPACK(DWORD, id);

	DWORD unknown;
	UNPACK(DWORD, unknown);

	BYTE type; // always 2?
	UNPACK(BYTE, type);

	if (type != 2)
	{
		DEBUGOUT("Unknown image type %u?\r\n", type);
		return FALSE;
	}

	DWORD numimages;
	UNPACK(DWORD, numimages);

	for (DWORD i = 0; i < numimages; i++)
	{
		DWORD imageid;
		UNPACK(DWORD, imageid);

#if PHATSDK_RENDER_AVAILABLE
		ImgColor *pColor = ImgColor::Get(imageid);

		if (pColor)
		{
			m_pD3DTexture9 = pColor->m_pD3DTexture9;
			m_pD3DTexture9->AddRef();
			break;
		}

		// why is ImgColor not being cleaned up here?
#endif
	}

	return TRUE;
}

// Replacements for deprecated
void ImgTex::UnlockTexture9()
{
#if PHATSDK_RENDER_AVAILABLE
	if (m_pD3DTexture9)
	{
		m_pD3DTexture9->UnlockRect(0);
		m_lPitch = 0;
	}
#endif
}

BOOL ImgTex::Load(ImgTex *pSource)
{
#if PHATSDK_RENDER_AVAILABLE
	if (m_lType != pSource->m_lType)
		return FALSE;
	if (m_lWidth != pSource->m_lWidth || m_lHeight != pSource->m_lHeight)
		return FALSE;

	if (!m_pD3DTexture9 || !pSource->m_pD3DTexture9)
		return FALSE;

	GetData();
	g_pD3DDevice->UpdateTexture(pSource->m_pD3DTexture9, m_pD3DTexture9);
	UnlockTexture9();

	// m_pD3DTexture9->Release();
	// m_pD3DTexture9 = pSource->m_pD3DTexture9;
	// m_pD3DTexture9->AddRef();

	// pSource->m_pD3DTexture9 = NULL;

	return TRUE;
#else
	return NULL;
#endif
}

ImgColor::ImgColor()
{
	/*
	m_dw18 = 0;
	m_dw1C = 0;
	m_dw20 = 0;
	m_dw24 = 0;

	m_lWidth = 0;
	m_lHeight = 0;
	m_lPitch = 0;
	m_pPixels = NULL;
	m_pPalette = NULL;
	m_bInCache = TRUE;
	m_pD3DTexture9 = NULL;

   #ifdef DEPRECATED_CODE
	m_pDDSurf = NULL;
	m_pSurfaceChunk = NULL;
   #endif
	*/

	m_Width = 0;
	m_Height = 0;
	m_Format = 0;
	m_Data = NULL;
	m_Length = 0;

#if PHATSDK_RENDER_AVAILABLE
	m_pD3DTexture9 = NULL;
#endif
}

ImgColor::~ImgColor()
{
	Destroy();
}

DBObj* ImgColor::Allocator()
{
	return((DBObj *)new ImgColor());
}

void ImgColor::Destroyer(DBObj* pImgColor)
{
	delete ((ImgTex *)pImgColor);
}

ImgColor *ImgColor::Get(DWORD ID)
{
	return (ImgColor *)ObjCaches::ImgColors->Get(ID);
}

void ImgColor::Destroy()
{
	if (m_Data)
	{
		delete[] m_Data;
		m_Data = NULL;
	}

#if PHATSDK_RENDER_AVAILABLE
	if (m_pD3DTexture9)
	{
		m_pD3DTexture9->Release();
		m_pD3DTexture9 = NULL;
	}
#endif
}

BOOL ImgColor::UnPack(BYTE **ppData, ULONG iSize)
{
	/*
	[16:22:20] <paradox> id/unk/width/height/format/length (all dword)
   [16:23:10] <paradox> format uses the D3DFMT_* enum for several types
   [16:23:34] <Pea> format needs to be translated
   [16:23:37] <Pea> or does it match
   [16:23:42] <paradox> format 1f4 is a jpeg which needs some special handling
	*/

	UNPACK(DWORD, id);

	DWORD unknown;
	UNPACK(DWORD, unknown);

	UNPACK(DWORD, m_Width);
	UNPACK(DWORD, m_Height);
	UNPACK(DWORD, m_Format);
	UNPACK(DWORD, m_Length);

	if (iSize < m_Length)
	{
		DEBUGOUT("Bad ImgColor data length.\n");
		return FALSE;
	}

	m_Data = new BYTE[m_Length];
	// if (m_Length < iSize)
	// __asm int 3;

	memcpy(m_Data, *ppData, m_Length);
	*ppData = *ppData + m_Length;
	iSize -= m_Length;

	SIZE szImage;
	if (m_Format == 0x1F4)
	{
		BYTE *pB = m_Data;
		for (int i = 0; i < 3; ++i)
		{
			for (; pB != (m_Data + m_Length); ++pB)
			{
				if ((pB[0] == 0xff) && (pB[1] == 0xc0))
				{
					pB += sizeof(BYTE) + 2 * sizeof(WORD);
					szImage.cy = MAKEWORD(pB[1], pB[0]);
					szImage.cx = MAKEWORD(pB[3], pB[2]);
					break;
				}
			}
		}
	}
	else
	{
		szImage.cx = m_Width;
		szImage.cy = m_Height;
	}

#if PHATSDK_RENDER_AVAILABLE

	IDirect3DSurface9 *pSurface;
	g_pD3DDevice->CreateOffscreenPlainSurface(szImage.cx, szImage.cy, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL);
	RECT rcDest = { 0, 0, szImage.cx, szImage.cy };
	RECT rcSrc = { 0, 0, szImage.cx, szImage.cy };

	switch (m_Format)
	{
		// Formats that dont look right
								///////////////////////////////
	case 0x00000065:
		{
			DWORD dwPalette;
			UNPACK(DWORD, dwPalette);
			// pStream->ReadBinary(sizeof(DWORD), (BYTE*) &dwPalette);

			// DWORD *pPalette = ReadPalette(dwPalette);
			Palette *pPalette = Palette::Get(dwPalette);

			D3DLOCKED_RECT lrect;
			pSurface->LockRect(&lrect, NULL, D3DLOCK_DISCARD);

			BYTE *pSource = reinterpret_cast <BYTE *> (m_Data);
			DWORD*pTarget = reinterpret_cast <DWORD *> (lrect.pBits);

			DWORD dwMax = 0;
			for (unsigned int i = 0; i < m_Height * m_Width; ++i)
			{
				WORD dwPix = MAKEWORD(pSource[0], pSource[1]);

				DWORD dwColor = pPalette->get_color_32(dwPix);

				BYTE bA = (BYTE)((dwColor & 0xFF000000) >> 24);
				BYTE bR = (BYTE)((dwColor & 0x00FF0000) >> 16);
				BYTE bG = (BYTE)((dwColor & 0x0000FF00) >> 8);
				BYTE bB = (BYTE)((dwColor & 0x000000FF) >> 0);
				pTarget[i] = D3DCOLOR_ARGB(bA, bR, bG, bB);
				/*
				BYTE bB = (BYTE) (((dwColor >> 10) & 0x1f) << 3);
								   BYTE bG = (BYTE) (((dwColor >> 5) & 0x1f) << 3);
								   BYTE bR = (BYTE) (((dwColor >> 0) & 0x1f) << 3);
								   BYTE bA = 0x00; // (BYTE) ((dwColor & 0x000000FF) >> 0);
				*/
				// pTarget[i] = D3DCOLOR_ARGB(bA, bR, bG, bB);

				pSource += 2;
			}

			pSurface->UnlockRect();

			Palette::releasePalette(pPalette);

			// delete[] pPalette;*/
// __asm int 3
			break;
		}

		//RAW 8bit PALETTE
	case 0x00000029:
		{
			/*
						   DWORD dwPalette;
						   pStream->ReadBinary(sizeof(DWORD), (BYTE*) &dwPalette);

						   DWORD*	pPalette = ReadPalette(dwPalette);

						   D3DLOCKED_RECT lrect;
						   pSurface->LockRect(&lrect, NULL, D3DLOCK_DISCARD);

						   DWORD*	pTarget = reinterpret_cast < DWORD * > (lrect.pBits);
						   for (unsigned int i = 0; i < header.m_dwY * header.m_dwX; ++i)
						   {
							   DWORD dwColor = pPalette[pMem[i] * 8];
							   BYTE r = (BYTE) ((dwColor & 0x00FF0000) >> 16);
							   BYTE g = (BYTE) ((dwColor & 0x0000FF00) >> 8);
							   BYTE b = (BYTE) ((dwColor & 0x000000FF) >> 0);
							   pTarget[i] = D3DCOLOR_ARGB((dwColor & 0xFF000000) >> 24, r, g, b);
						   }

						   pSurface->UnlockRect();

						   delete[] pPalette;*/
			DebugBreak();
			break;
		}

		//RAW 8bit GS Alpha/Numeric maps
	case 0x0000001C:
		//RAW 8bit GS
	case 0x000000f4:
		D3DXLoadSurfaceFromMemory(pSurface, NULL, &rcDest, m_Data, D3DFMT_L8, 1 * m_Width, NULL, &rcSrc, D3DX_FILTER_NONE, 0xFF000000);
		break;

		//RAW B8G8R8
	case 0x000000f3:
		{
			for (unsigned int i = 0; i < m_Height * m_Width * 3; i += 3)
			{
				BYTE r = m_Data[i];
				BYTE b = m_Data[i + 2];
				m_Data[i] = b;
				m_Data[i + 2] = r;
			}

			D3DXLoadSurfaceFromMemory(pSurface, NULL, &rcDest, m_Data, D3DFMT_R8G8B8, 3 * m_Width, NULL, &rcSrc, D3DX_FILTER_NONE, 0xFF000000);
			break;
		}

		//JPEG
	case 0x000001f4:
		D3DXLoadSurfaceFromFileInMemory(pSurface, NULL, &rcDest, m_Data, m_Length, &rcSrc, D3DX_FILTER_NONE, 0xFF000000, NULL);
		break;

	case D3DFMT_DXT1:		//0x01545844
		D3DXLoadSurfaceFromMemory(pSurface, NULL, &rcDest, m_Data, (D3DFORMAT)m_Format, 2 * m_Width, NULL, &rcSrc, D3DX_FILTER_NONE, 0xFF000000);
		break;

	case D3DFMT_DXT5:		//0x05545844
	case D3DFMT_A8R8G8B8:	//0x00000015
		D3DXLoadSurfaceFromMemory(pSurface, NULL, &rcDest, m_Data, (D3DFORMAT)m_Format, 4 * m_Width, NULL, &rcSrc, D3DX_FILTER_NONE, 0xFF000000);
		break;

	case D3DFMT_R8G8B8:		//0x00000014
		D3DXLoadSurfaceFromMemory(pSurface, NULL, &rcDest, m_Data, (D3DFORMAT)m_Format, 3 * m_Width, NULL, &rcSrc, D3DX_FILTER_NONE, 0xFF000000);
		break;

	default:
		D3DXLoadSurfaceFromMemory(pSurface, NULL, &rcDest, m_Data, (D3DFORMAT)m_Format, 4 * m_Width, NULL, &rcSrc, D3DX_FILTER_NONE, 0xFF000000);
	}

	// delete[] m_Data;
	// pStream.Release();

	IDirect3DTexture9 *pTexture;
	HRESULT hr = D3DXCreateTexture(g_pD3DDevice, szImage.cx, szImage.cy, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture);
	if (hr != D3D_OK)
	{
		pSurface->Release();
		DEBUGOUT("Failed to create texture: 0x%08x\n", hr);
		return FALSE;
	}

	IDirect3DSurface9 *pTexSurf;
	pTexture->GetSurfaceLevel(0, &pTexSurf);
	D3DXLoadSurfaceFromSurface(pTexSurf, NULL, &rcDest, pSurface, NULL, &rcSrc, D3DX_FILTER_NONE, 0xFF000000);
	pTexSurf->Release();
	pSurface->Release();

	DEBUGOUT("Success with texture: 0x%08x\n", GetID());
	m_pD3DTexture9 = pTexture;
#endif

	return TRUE;
}

#endif
