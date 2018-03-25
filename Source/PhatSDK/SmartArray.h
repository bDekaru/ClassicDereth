
#pragma once

#define SMARTARRAY_DEFAULT_SIZE 8

template<class T>
class SmartArray : public PackObj
{
public:
	SmartArray(const SmartArray& other);
    SmartArray(long initial_size);
    SmartArray();
    ~SmartArray();

	SmartArray<T> &operator =(const SmartArray<T>& other);

    void add(T *pdata);
    BOOL grow(long new_size);
    BOOL RemoveUnOrdered(T *pdata);

	bool UnPack(BinaryReader *pReader) override; // Custom
	void Pack(BinaryWriter *pWriter) override; // Custom
	bool UnPackPackObj(BinaryReader *pReader); // Custom
	void PackPackObj(BinaryWriter *pWriter); // Custom
	bool UnSerialize(BinaryReader *pReader); // Custom
	bool UnSerializePackObj(BinaryReader *pReader); // Custom

    T *array_data;
    long num_alloc;
    long num_used;
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

	for (DWORD i = 0; i < num_used; i++)
		array_data[i] = other.array_data[i];

	return *this;
}

template<class T>
SmartArray<T>::SmartArray(long initial_size)
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
		delete [] array_data;
}

template<class T>
bool SmartArray<T>::UnPack(BinaryReader *pReader)
{
	DWORD numElements = pReader->Read<DWORD>();

	if (array_data)
		delete [] array_data;

	array_data = new T[numElements];
	for (DWORD i = 0; i < numElements; i++)
		array_data[i] = pReader->Read<T>();

	num_used = numElements;
	num_alloc = numElements;
	return true;
}

template<class T>
void SmartArray<T>::Pack(BinaryWriter *pWriter)
{
	pWriter->Write<DWORD>(num_used);
	pWriter->Write(array_data, num_used * sizeof(T));
}

template<class T>
bool SmartArray<T>::UnPackPackObj(BinaryReader *pReader)
{
	DWORD numElements = pReader->Read<DWORD>();

	if (array_data)
		delete[] array_data;

	array_data = new T[numElements];
	for (DWORD i = 0; i < numElements; i++)
		array_data[i].UnPack(pReader);

	num_used = numElements;
	num_alloc = numElements;
	return true;
}

template<class T>
void SmartArray<T>::PackPackObj(BinaryWriter *pWriter)
{
	pWriter->Write<DWORD>(num_used);
	for (DWORD i = 0; i < num_used; i++)
		array_data[i].Pack(pWriter);
}

template<class T>
bool SmartArray<T>::UnSerialize(BinaryReader *pReader)
{
	DWORD numElements = pReader->ReadCompressedUInt32();

	if (array_data)
		delete[] array_data;

	array_data = new T[numElements];
	for (DWORD i = 0; i < numElements; i++)
		array_data[i] = pReader->Read<T>();

	num_used = numElements;
	num_alloc = numElements;
	return true;
}

template<class T>
bool SmartArray<T>::UnSerializePackObj(BinaryReader *pReader)
{
	DWORD numElements = pReader->ReadCompressedUInt32();

	if (array_data)
		delete[] array_data;

	array_data = new T[numElements];
	for (DWORD i = 0; i < numElements; i++)
		array_data[i].UnPack(pReader);

	num_used = numElements;
	num_alloc = numElements;
	return true;
}

template<class T>
BOOL SmartArray<T>::grow(long new_size)
{
    if (new_size < num_used)
        return FALSE;

    if (new_size > num_alloc)
    {
        if (!new_size)
        {
            num_used = 0;
            num_alloc = 0;

            delete [] array_data;
            array_data = NULL;
        }
        else
        {
            T *new_data = new T[ new_size ];

            if (!new_data)
                return FALSE;

            if (array_data)
            {
                for (int i = 0; i < num_used; i++)
                    new_data[i] = array_data[i];

                delete [] array_data;
            }

            array_data    = new_data;
            num_alloc    = new_size;
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
    for (long i = 0; i < num_used; i++)
    {
        if (array_data[i] == *pdata)
        {
            array_data[i] = array_data[ num_used - 1 ];
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
    OldSmartArray(long grow_step = 2);
    ~OldSmartArray();

    void Add(T *pdata);
    void Grow(long new_size);

    T *array_data;
    long grow_size;
    long num_alloc;
    long num_used;
};

template<class T>
OldSmartArray<T>::OldSmartArray(long grow_step)
{
    array_data = NULL;
    grow_size = grow_step;
    num_used = 0;
    num_alloc = 0;
}

template<class T>
OldSmartArray<T>::~OldSmartArray()
{
    delete [] array_data;
}

template<class T>
void OldSmartArray<T>::Grow(long new_size)
{
    T *new_data = new T[ new_size ];

    if (!new_data)
        return;

    if (array_data)
    {
        for (int i = 0; i < num_used; i++)
            new_data[i] = array_data[i];

        delete [] array_data;
    }

    array_data    = new_data;
    num_alloc    = new_size;
}

template<class T>
void OldSmartArray<T>::Add(T *pdata)
{
    if (num_used >= num_alloc)
        Grow(num_alloc + grow_size);

    array_data[num_used] = *pdata;
    num_used++;
}

