
#pragma once

template<class T>
class DArray
{
public:
	DArray(DWORD initial_size, DWORD grow_size);
	DArray(DWORD initial_size);
	~DArray();

	void ensure_space(DWORD new_size, DWORD grow_size);
	void safe_add(T *pdata, DWORD index);
	void shrink(DWORD new_size);
	void grow(DWORD new_size);
	void remove_index(unsigned int index);

	DWORD GetMaxCount();

	union {
		T *array_data; // 0
		T *data; // 0
	};
	DWORD grow_size; // 4
	DWORD num_used; // 8
	DWORD alloc_size; // 0xC
};

template<class T>
DArray<T>::DArray(DWORD initial_size, DWORD grow_size)
{
	array_data = new T[initial_size];
	grow_size = grow_size;
	num_used = 0;
	alloc_size = initial_size;
}

template<class T>
DArray<T>::DArray(DWORD grow_size)
{
	array_data = new T[0]; // NULL;
	grow_size = grow_size;
	num_used = 0;
	alloc_size = 0;
}

template<class T>
DArray<T>::~DArray()
{
	delete[] array_data;
}

template<class T>
void DArray<T>::ensure_space(DWORD new_size, DWORD grow_size)
{
	if (new_size > alloc_size)
		grow(alloc_size + grow_size);
}

template<class T>
void DArray<T>::grow(DWORD new_size)
{
	if (new_size > alloc_size)
	{
		T *new_data = new T[new_size];

		for (DWORD i = 0; i < alloc_size; i++)
			new_data[i] = array_data[i];

		delete[] array_data;

		array_data = new_data;
		alloc_size = new_size;
	}
	else
		shrink(new_size);
}

template<class T>
void DArray<T>::shrink(DWORD new_size)
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

			for (DWORD i = 0; i < new_size; i++)
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
void DArray<T>::safe_add(T *pdata, DWORD index)
{
	if (index >= alloc_size)
		grow(grow_size + index);

	array_data[index] = *pdata;

	if (index > num_used)
		num_used = index + 1;
}

