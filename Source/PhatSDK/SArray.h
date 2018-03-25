
#pragma once

template<class T>
class SArray
{
public:
    SArray();
    ~SArray();

    void safe_add(T *pdata, DWORD index);
    void shrink(DWORD new_size);
    void grow(DWORD new_size);

    DWORD GetMaxCount();

	union
	{
		T *array_data;
		T *data;
	};
    DWORD array_size;
};

template<class T>
SArray<T>::SArray()
{
    array_data = NULL;
    array_size = 0;
}

template<class T>
SArray<T>::~SArray()
{
    delete [] array_data;
}

template<class T>
void SArray<T>::grow(DWORD new_size)
{
    if (new_size > array_size)
    {
        T *new_data = new T[ new_size ];

        for (DWORD i = 0; i < array_size; i++)
            new_data[i] = array_data[i];

        if (array_data)
            delete [] array_data;

        array_data = new_data;
        array_size = new_size;
    }
    else
        shrink(new_size);
}

template<class T>
void SArray<T>::shrink(DWORD new_size)
{
    if (new_size <= array_size)
    {
        if (!new_size)
        {
            delete [] array_data;
            array_size = 0;
            array_data = NULL;
        }
        else
        {
            T *new_data = new T[ new_size ];

            for (DWORD i = 0; i < new_size; i++)
                new_data[i] = array_data[i];

            if (array_data)
                delete [] array_data;

            array_data = new_data;
            array_size = new_size;
        }
    }
    else
        grow(new_size);
}
