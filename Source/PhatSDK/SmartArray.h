
#pragma once

#define SMARTARRAY_DEFAULT_SIZE 8

template<class T>
class SmartArray : public PackObj
{
public:
	SmartArray(const SmartArray& other);
	SmartArray(int32_t initial_size);
	SmartArray();
	~SmartArray();

	SmartArray<T> &operator =(const SmartArray<T>& other);

	void add(T *pdata);
	BOOL grow(int32_t new_size);
	BOOL RemoveUnOrdered(T *pdata);

	bool UnPackPackObj(BinaryReader *pReader); // Custom
	void PackPackObj(BinaryWriter *pWriter); // Custom
	bool UnSerialize(BinaryReader *pReader); // Custom
	bool UnSerializePackObj(BinaryReader *pReader); // Custom

	T *array_data;
	int32_t num_alloc;
	int32_t num_used;

public:
	bool UnPack(BinaryReader *pReader) override
	{
		return UnPackInternal(pReader);
	}

	void Pack(BinaryWriter *pWriter) override
	{
		PackInternal(pWriter);
	}

protected:

	bool UnPackInternal(BinaryReader *pReader)
	{
		uint32_t numElements = pReader->Read<uint32_t>();

		if (array_data)
			delete[] array_data;

		array_data = new T[numElements];
		for (uint32_t i = 0; i < numElements; i++)
			array_data[i] = pReader->Read<T>();

		num_used = numElements;
		num_alloc = numElements;
		return true;
	}

	// not string, not packable
	template<class value_t = T>
	typename std::enable_if_t<
		!(std::is_same_v<std::string, value_t> || std::is_same_v<std::u16string, value_t>
			|| std::is_base_of_v<PackObj, value_t>)>
		PackInternal(BinaryWriter *pWriter)
	{
		pWriter->Write<uint32_t>(num_used);
		pWriter->Write(array_data, num_used * sizeof(T));
	}

	// string, not packable
	template<class value_t = T>
	typename std::enable_if_t<std::is_same_v<std::string, value_t> || std::is_same_v<std::u16string, value_t>>
		PackInternal(BinaryWriter *pWriter)
	{
		pWriter->Write<uint32_t>(num_used);
		for (int i = 0; i < num_used; i++)
			pWriter->WriteString(array_data[i]);
	}

	// packable
	template<class value_t = T>
	typename std::enable_if_t<std::is_base_of_v<PackObj, value_t>>
		PackInternal(BinaryWriter *pWriter)
	{
		pWriter->Write<uint32_t>(num_used);
		for (int i = 0; i < num_used; i++)
			array_data[i].Pack(pWriter);
	}
};

template<class T>
SmartArray<T>::SmartArray(const SmartArray<T>& other)
{
	*this = other;
}

template<class T>
SmartArray<T> &SmartArray<T>::operator =(const SmartArray<T>& other)
{
	num_used = other.num_used;
	num_alloc = other.num_used;
	array_data = new T[num_used];

	for (uint32_t i = 0; i < num_used; i++)
		array_data[i] = other.array_data[i];

	return *this;
}

template<class T>
SmartArray<T>::SmartArray(int32_t initial_size)
{
	array_data = NULL;
	num_used = 0;
	num_alloc = 0;

	grow(initial_size);
}

template<class T>
SmartArray<T>::SmartArray()
{
	array_data = NULL;
	num_used = 0;
	num_alloc = 0;
}

template<class T>
SmartArray<T>::~SmartArray()
{
	if (array_data)
		delete[] array_data;
}

template<class T>
bool SmartArray<T>::UnPackPackObj(BinaryReader *pReader)
{
	uint32_t numElements = pReader->Read<uint32_t>();

	if (array_data)
		delete[] array_data;

	array_data = new T[numElements];
	for (uint32_t i = 0; i < numElements; i++)
		array_data[i].UnPack(pReader);

	num_used = numElements;
	num_alloc = numElements;
	return true;
}

template<class T>
void SmartArray<T>::PackPackObj(BinaryWriter *pWriter)
{
	pWriter->Write<uint32_t>(num_used);
	for (uint32_t i = 0; i < num_used; i++)
		array_data[i].Pack(pWriter);
}

template<class T>
bool SmartArray<T>::UnSerialize(BinaryReader *pReader)
{
	uint32_t numElements = pReader->ReadCompressedUInt32();

	if (array_data)
		delete[] array_data;

	array_data = new T[numElements];
	for (uint32_t i = 0; i < numElements; i++)
		array_data[i] = pReader->Read<T>();

	num_used = numElements;
	num_alloc = numElements;
	return true;
}

template<class T>
bool SmartArray<T>::UnSerializePackObj(BinaryReader *pReader)
{
	uint32_t numElements = pReader->ReadCompressedUInt32();

	if (array_data)
		delete[] array_data;

	array_data = new T[numElements];
	for (uint32_t i = 0; i < numElements; i++)
		array_data[i].UnPack(pReader);

	num_used = numElements;
	num_alloc = numElements;
	return true;
}

template<class T>
BOOL SmartArray<T>::grow(int32_t new_size)
{
	if (new_size < num_used)
		return FALSE;

	if (new_size > num_alloc)
	{
		if (!new_size)
		{
			num_used = 0;
			num_alloc = 0;

			delete[] array_data;
			array_data = NULL;
		}
		else
		{
			T *new_data = new T[new_size];

			if (!new_data)
				return FALSE;

			if (array_data)
			{
				for (int i = 0; i < num_used; i++)
					new_data[i] = array_data[i];

				delete[] array_data;
			}

			array_data = new_data;
			num_alloc = new_size;
		}
	}

	return TRUE;
}

template<class T>
void SmartArray<T>::add(T *pdata)
{
	if (num_used >= num_alloc)
	{
		if (num_alloc > 0)
		{
			// Double the size
			if (!grow(num_alloc * 2))
				return;
		}
		else
		{
			// Default size is 8
			if (!grow(SMARTARRAY_DEFAULT_SIZE))
				return;
		}
	}

	array_data[num_used] = *pdata;
	num_used++;
}

template<class T>
BOOL SmartArray<T>::RemoveUnOrdered(T *pdata)
{
	for (int32_t i = 0; i < num_used; i++)
	{
		if (array_data[i] == *pdata)
		{
			array_data[i] = array_data[num_used - 1];
			num_used--;
			return TRUE;
		}
	}

	return FALSE;
}

template<class T>
class OldSmartArray
{
public:
	OldSmartArray(int32_t grow_step = 2);
	~OldSmartArray();

	void Add(T *pdata);
	void Grow(int32_t new_size);

	T *array_data;
	int32_t grow_size;
	int32_t num_alloc;
	int32_t num_used;
};

template<class T>
OldSmartArray<T>::OldSmartArray(int32_t grow_step)
{
	array_data = NULL;
	grow_size = grow_step;
	num_used = 0;
	num_alloc = 0;
}

template<class T>
OldSmartArray<T>::~OldSmartArray()
{
	delete[] array_data;
}

template<class T>
void OldSmartArray<T>::Grow(int32_t new_size)
{
	T *new_data = new T[new_size];

	if (!new_data)
		return;

	if (array_data)
	{
		for (int i = 0; i < num_used; i++)
			new_data[i] = array_data[i];

		delete[] array_data;
	}

	array_data = new_data;
	num_alloc = new_size;
}

template<class T>
void OldSmartArray<T>::Add(T *pdata)
{
	if (num_used >= num_alloc)
		Grow(num_alloc + grow_size);

	array_data[num_used] = *pdata;
	num_used++;
}

