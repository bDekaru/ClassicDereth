
#pragma once

#include "BinaryReader.h"

class LegacyPackObj
{
public:
	LegacyPackObj();
	virtual ~LegacyPackObj();

	// Use both names? One will call the other?
	virtual ULONG GetPackSize();
	virtual ULONG pack_size();

	// The main packing calls.
	virtual ULONG Pack(BYTE** ppData, ULONG iSize) { return 0; }
	virtual BOOL UnPack(BYTE** ppData, ULONG iSize) { return TRUE; };

	// Packing functions
	static BOOL ALIGN_PTR(BYTE** ppData, ULONG* piSize);

	template< typename ValType, typename OutputType >
	static BOOL UNPACK_TYPE(OutputType* pBuffer, BYTE** ppData, ULONG* piSize)
	{
		if ((*piSize) < sizeof(ValType))
			return FALSE;

		*pBuffer = (OutputType)(*((ValType *)*ppData));
		*ppData = (*ppData) + sizeof(ValType);
		*piSize = (*piSize) - sizeof(ValType);

		return TRUE;
	}

	template< typename ValType, typename InputType >
	static BOOL PACK_TYPE(InputType* pBuffer, BYTE** ppData, ULONG* piSize)
	{
		if ((*piSize) < sizeof(ValType))
			return FALSE;

		*((ValType *)(*ppData)) = *pBuffer;
		*ppData = (*ppData) + sizeof(ValType);
		*piSize = (*piSize) - sizeof(ValType);

		return TRUE;
	}
};

inline bool UnPackCompressed32(DWORD *output, BYTE **ppData, ULONG *piSize)
{
	BinaryReader reader(*ppData, *piSize);
	*output = reader.ReadCompressedUInt32();

	if (reader.GetLastError())
		return false;

	DWORD numRead = reader.GetOffset();
	if (numRead > *piSize)
		numRead = *piSize;

	*ppData = *ppData + numRead;
	*piSize = *piSize - numRead;
	return true;
}

// UnPack Variables
#define UNPACK(type, output_var) LegacyPackObj::UNPACK_TYPE< type >(&output_var, ppData, &iSize)
#define UNPACK_POBJ(pobj) pobj->UnPack(ppData, iSize)
#define UNPACK_OBJ(obj) obj.UnPack(ppData, iSize)
#define UNPACK_COMPRESSED32(output_var) UnPackCompressed32(&output_var, ppData, &iSize)
#define PACK(type, input_var) LegacyPackObj::PACK_TYPE< type >(&input_var, ppData, &iSize)
#define PACK_POBJ(pobj) pobj->Pack(ppData, iSize)
#define PACK_OBJ(obj) obj.Pack(ppData, iSize)
#define PACK_ALIGN() LegacyPackObj::ALIGN_PTR(ppData, &iSize)

#define FBitSet(flags, bit) ((((DWORD)flags) >> (bit)) & 1)

template<typename T>
inline BOOL UnpackObjWithReader(T &obj, BYTE** ppData, ULONG iSize)
{
	BinaryReader reader(*ppData, iSize);
	BOOL success = obj.UnPack(&reader) ? TRUE : FALSE;

	DWORD numRead = reader.GetOffset();
	if (numRead > iSize)
		numRead = iSize;

	*ppData = *ppData + numRead;
	iSize = iSize - numRead;
	return success;
}

#define UNPACK_OBJ_READER(obj) UnpackObjWithReader(obj, ppData, iSize)

template<typename T>
inline BOOL PackObjWithWriter(T &obj, BYTE** ppData, ULONG iSize)
{
	BinaryWriter writer;
	obj.Pack(&writer);

	memcpy(*ppData, writer.GetData(), writer.GetSize());
	*ppData = *ppData + writer.GetSize();
	iSize -= writer.GetSize();

	return TRUE;
}

#define PACK_OBJ_WRITER(obj) PackObjWithWriter(obj, ppData, iSize)






