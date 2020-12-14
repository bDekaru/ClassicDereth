
#pragma once

template<class T>
class DArray
{
public:
	DArray(uint32_t initial_size, uint32_t grow_size);
	DArray(uint32_t initial_size);
	virtual ~DArray();

	void ensure_space(uint32_t new_size, uint32_t grow_size);
	void safe_add(T *pdata, uint32_t index);
	void shrink(uint32_t new_size);
	void grow(uint32_t new_size);
	void remove_index(unsigned int index);

	uint32_t GetMaxCount();

	union {
		T *array_data; // 0
		T *data; // 0
	};
	uint32_t grow_size; // 4
	uint32_t num_used; // 8
	uint32_t alloc_size; // 0xC
};

template<class T>
DArray<T>::DArray(uint32_t initial_size, uint32_t grow_size) :
	array_data(nullptr), grow_size(grow_size), num_used(0), alloc_size(initial_size)
{
	if (initial_size > 0)
		array_data = new T[initial_size];
}

template<class T>
DArray<T>::DArray(uint32_t grow_size) :
	DArray(0, grow_size)
{
}

template<class T>
DArray<T>::~DArray()
{
	if (array_data)
		delete[] array_data;
}

template<class T>
void DArray<T>::ensure_space(uint32_t new_size, uint32_t grow_size)
{
	if (new_size > alloc_size)
		grow(alloc_size + grow_size);
}

template<class T>
void DArray<T>::grow(uint32_t new_size)
{
	if (new_size > alloc_size)
	{
		T *new_data = new T[new_size];

		for (uint32_t i = 0; i < alloc_size; i++)
			new_data[i] = array_data[i];

		delete[] array_data;

		array_data = new_data;
		alloc_size = new_size;
	}
	else
		shrink(new_size);
}

template<class T>
void DArray<T>::shrink(uint32_t new_size)
{
	if (new_size <= alloc_size)
	{
		if (!new_size)
		{
			delete[] array_data;
			array_data = NULL;
			alloc_size = 0;
		}
		else
		{
			T *new_data = new T[new_size];

			for (uint32_t i = 0; i < new_size; i++)
				new_data[i] = array_data[i];

			delete[] array_data;

			array_data = new_data;
			alloc_size = new_size;
		}

		if (num_used > alloc_size)
			num_used = alloc_size;
	}
	else
		grow(new_size);
}

template<class T>
void DArray<T>::safe_add(T *pdata, uint32_t index)
{
	if (index >= alloc_size)
		grow(grow_size + index);

	array_data[index] = *pdata;

	if (index > num_used)
		num_used = index + 1;
}

