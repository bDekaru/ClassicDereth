
#pragma once
#include "easylogging++.h"

template<typename _Kty> class HashBase;
template<typename _Kty> class HashBaseData;
template<typename _Kty> class HashBaseIter;

template<class _Mty> class LongNIValHash;
template<class _Mty> class LongNIValHashData;
template<class _Mty> class LongNIValHashIter;

template<class _Mty> class LongNIHash;
template<class _Mty> class LongNIHashData;
template<class _Mty> class LongNIHashIter;

template<typename _Kty>
class HashBaseData
{
public:
	HashBaseData() : hash_next(NULL), id(0)
	{
	}

	virtual ~HashBaseData()
	{
		hash_next = NULL;
		id = 0;
	}

	 _Kty GetID()
	{
		return id;
	}

	void SetID(const _Kty new_id)
	{
		assert(hash_next == NULL); // don't change ID when already inserted!
		id = new_id;
	}

	HashBaseData<_Kty> *hash_next;
	_Kty id;
};

template<typename _Kty>
class HashBaseIter
{
public:

	HashBaseIter(HashBase<_Kty> *pBase) : m_pBase(pBase)
	{
		m_pBase = pBase;

		SetBegin();
	}

	void SetBegin()
	{
		m_iBucket = 0;
		m_pLast = NULL;

		if (m_pBase)
		{
			m_pNodePtr = m_pBase->GetBucket(0);
			m_bEndReached = FALSE;

			if (!m_pNodePtr)
				Next();
		}
		else
		{
			m_pNodePtr = NULL;
			m_bEndReached = TRUE;
		}
	}

	void Next()
	{
		// Grab the next node from this bucket.
		if (m_pNodePtr)
		{
			m_pLast = m_pNodePtr;
			m_pNodePtr = m_pNodePtr->hash_next;
		}

		while (!m_pNodePtr)
		{
			// Dead node. Use the next bucket.
			m_iBucket++;
			m_pLast = NULL;

			if (m_iBucket>= m_pBase->GetBucketCount())
			{
				// The end has been reached.
				m_bEndReached = TRUE;
				return;
			}

			m_pNodePtr = m_pBase->GetBucket(m_iBucket);
		}
	}

	void DeleteCurrent()
	{
		if (EndReached())
			return;

		HashBaseData<_Kty> *pNextNode;

		if (m_pLast)
		{
			// This current node was not the base of a bucket.
			// To remove it from the chain, just link around it.
			m_pLast->hash_next = m_pNodePtr->hash_next;

			// This will be our next node.
			pNextNode = m_pLast->hash_next;
		}
		else
		{
			// Going to have to change the bucket base.
			m_pBase->SetBucket(m_iBucket, m_pNodePtr->hash_next);

			// This will be our next node.
			pNextNode = m_pBase->GetBucket(m_iBucket);
		}

		// This check is useless, but Turbine does it. Probably inlined.
		if (m_pNodePtr)
			delete m_pNodePtr;

		while (!pNextNode)
		{
			// Dead node. Use the next bucket.
			m_iBucket++;
			m_pLast = NULL;

			if (m_iBucket>= m_pBase->GetBucketCount())
			{
				// The end has been reached.
				m_bEndReached = TRUE;
				m_pNodePtr = NULL;
				return;
			}

			pNextNode = m_pBase->GetBucket(m_iBucket);
		}

		m_pNodePtr = pNextNode;
	}

	// -- Pea you made this one up. It was probably inlined. --
	HashBaseData<_Kty> *GetCurrent()
	{
		return m_pNodePtr;
	}

	// -- Pea you made this one up. It was probably inlined. --
	BOOL EndReached()
	{
		return m_bEndReached;
	}

private:

	HashBase<_Kty> *m_pBase; // 0x00
	size_t m_iBucket; // 0x04
	HashBaseData<_Kty> *m_pNodePtr; // 0x08
	HashBaseData<_Kty> *m_pLast; // 0x0C
	BOOL m_bEndReached; // 0x10
};

template<typename _Kty>
class HashBase
{
public:
	HashBase(size_t buckets)
	{
		preserve_table = FALSE;
		hash_table = new HashBaseData<_Kty>*[buckets];

		InternalInit(buckets);
	}

	virtual ~HashBase()
	{
		if (!preserve_table)
			delete[] hash_table;
	}

	void InternalInit(size_t buckets)
	{
		bucket_count = buckets;

		// 64-bit = 16-bit shift
		// 32-bit = 8-bit shift
		hash_shift = sizeof(_Kty) <<1;

		// Calculate hash mask.
		hash_mask = 0;

		for (unsigned int bit = 1; (hash_mask | bit) <bucket_count; bit = bit <<1)
			hash_mask |= bit;

		// Init the table.
		for (unsigned int i = 0; i <bucket_count; i++)
			hash_table[i] = NULL;
	}

	HashBaseData<_Kty> *remove(_Kty Key)
	{
		size_t BucketIndex = (Key ^ (Key>> hash_shift)) & hash_mask;

		HashBaseData<_Kty> *pBucket = hash_table[BucketIndex];

		if (!pBucket)
			return NULL;

		if (pBucket->id == Key)
		{
			// First entry of bucket was a match.
			hash_table[BucketIndex] = pBucket->hash_next;
			return pBucket;
		}

		while (pBucket->hash_next)
		{
			if (pBucket->hash_next->id == Key)
			{
				HashBaseData<_Kty> *pEntry = pBucket->hash_next;

				// Remove from chain.
				pBucket->hash_next = pEntry->hash_next;

				return pEntry;
			}

			pBucket = pBucket->hash_next;
		}

		// No match found.
		return NULL;
	}

	void add(HashBaseData<_Kty> *pData)
	{
		size_t BucketIndex = (pData->id ^ (pData->id>> hash_shift)) & hash_mask;

		pData->hash_next = hash_table[BucketIndex];
		hash_table[BucketIndex] = pData;
	}

	HashBaseData<_Kty> *lookup(_Kty Key)
	{
		HashBaseData<_Kty> *pBucket = hash_table[(Key ^ (Key>> hash_shift)) & hash_mask];

		while (pBucket)
		{
			if (pBucket->id == Key)
				return pBucket;

			pBucket = pBucket->hash_next;
		}

		return NULL;
	}

	HashBaseData<_Kty> *clobber(HashBaseData<_Kty> *pData)
	{
		size_t BucketIndex = (pData->id ^ (pData->id>> hash_shift)) & hash_mask;

		HashBaseData<_Kty> *pBucket, *pBaseBucket, *pLastBucket;
		pBaseBucket = pBucket = hash_table[BucketIndex];
		pLastBucket = NULL;

		if (!pBucket)
		{
			hash_table[BucketIndex] = pData;
			return NULL;
		}

		while (pData->id != pBucket->id)
		{
			pLastBucket = pBucket;
			pBucket = pBucket->hash_next;

			if (!pBucket)
			{
				// Add me, I'm a unique data entry.
				pData->hash_next = pBaseBucket;
				hash_table[BucketIndex] = pData;

				// No data entry existed under my key.
				return NULL;
			}
		}

		if (pData != pBucket)
		{
			// Replace the existing data (It's using my key!).

			if (!pLastBucket)
			{
				// The duplicate is the base bucket.
				// To remove it, link the next and change the base.
				pData->hash_next = pBucket->hash_next;
				hash_table[BucketIndex] = pData;
			}
			else
			{
				// The duplicate is not the base bucket.
				// To remove it, just link around it.
				pLastBucket->hash_next = pData;
				pData->hash_next = pBucket->hash_next;
			}
		}

		// Return the data that was already in the table.
		return pBucket;
	}

	// -- Pea you made this one up. It was probably inlined. --
	HashBaseData<_Kty> **GetTable()
	{
		return hash_table;
	}

	// -- Pea you made this one up. It was probably inlined. --
	size_t GetBucketIndex(_Kty Key)
	{
		return((Key ^ (Key>> hash_shift)) & hash_mask);
	}

	// -- Pea you made this one up. It was probably inlined. --
	HashBaseData<_Kty> *GetBucket(size_t index)
	{
		return hash_table[index];
	}

	// -- Pea you made this one up. It was probably inlined. --
	size_t GetBucketCount()
	{
		return bucket_count;
	}

	// -- Pea you made this one up. It was probably inlined. --
	void SetBucket(size_t index, HashBaseData<_Kty> *pData)
	{
		hash_table[index] = pData;
	}

private:
	_Kty hash_mask; // 0x04 (assuming 32-bit..)
	UINT hash_shift; // 0x08 
	HashBaseData<_Kty> **hash_table; // 0x0C
	size_t bucket_count; // 0x10
	BOOL preserve_table; // 0x14
};

class LongHashData : public HashBaseData<uint32_t>
{
public:
	virtual ~LongHashData() { }
};

template<class _Mty>
class LongHash : public HashBase<uint32_t>
{
public:
	typedef HashBase<uint32_t> _Mybase;
	typedef HashBaseData<uint32_t> _MybaseData;
	typedef HashBaseIter<uint32_t> _MybaseIter;

	LongHash(size_t buckets)
		: _Mybase(buckets)
	{
	}

	virtual ~LongHash()
	{
		destroy_contents();
	}

	void add(_Mty *pData)
	{
		_Mybase::add(pData);
	}

	void remove(uint32_t Key)
	{
		_Mybase::remove(Key);
	}

	_Mty *lookup(uint32_t Key)
	{
		// We are converting the data pointers to their parent objects.
		_MybaseData *pData = _Mybase::lookup(Key);

		if (pData) // Safe ?
			return(static_cast<_Mty*>(pData));

		return NULL;
	}

	void destroy_contents()
	{
		try
		{
			_MybaseIter it(this);
			try
			{
				while (!it.EndReached())
					it.DeleteCurrent();
			}
			catch (...)
			{
				SERVER_ERROR << "Error progressing over iteration";
			}

		}
		catch (...)
		{
			SERVER_ERROR << "Failed to create iterator";
		}

	}
};

template<class _Mty>
class LongNIValHashData : public HashBaseData<uint32_t>
{
public:
	typedef HashBaseData<uint32_t> _Mybase;

	LongNIValHashData(_Mty Data, uint32_t Key) : m_Data(Data)
	{
		id = Key;
	}

	virtual ~LongNIValHashData()
	{
	}

	_Mty m_Data;
};

template<typename _Mty>
class LongNIValHashIter
{
public:
	typedef LongNIValHash<_Mty> _MyHash;
	typedef LongNIValHashData<_Mty> _MyData;
	typedef LongNIValHashIter<_Mty> _MyIter;

	LongNIValHashIter(_MyHash *pBase) : m_pBase(pBase)
	{

		m_iBucket = 0;
		m_pLast = NULL;

		if (m_pBase)
		{
			m_pNodePtr = static_cast<_MyData *>(m_pBase->GetBucket(0));
			m_bEndReached = FALSE;

			if (!m_pNodePtr)
				Next();
		}
		else
		{
			m_pNodePtr = NULL;
			m_bEndReached = TRUE;
		}
	}

	void Next()
	{
		// Grab the next node from this bucket.
		if (m_pNodePtr)
		{
			m_pLast = m_pNodePtr;
			m_pNodePtr = static_cast<_MyData *>(m_pNodePtr->hash_next);
		}

		while (!m_pNodePtr)
		{
			// Dead node. Use the next bucket.
			m_iBucket++;
			m_pLast = NULL;

			if (m_iBucket>= m_pBase->GetBucketCount())
			{
				// The end has been reached.
				m_bEndReached = TRUE;
				return;
			}

			m_pNodePtr = static_cast<_MyData *>(m_pBase->GetBucket(m_iBucket));
		}
	}

	void DeleteCurrent()
	{
		if (EndReached())
			return;

		_MyData *pNextNode;

		if (m_pLast)
		{
			// This current node was not the base of a bucket.
			// To remove it from the chain, just link around it.
			m_pLast->hash_next = m_pNodePtr->hash_next;

			// This will be our next node.
			pNextNode = static_cast<_MyData *>(m_pLast->hash_next);
		}
		else
		{
			// Going to have to change the bucket base.
			m_pBase->SetBucket(m_iBucket, m_pNodePtr->hash_next);

			// This will be our next node.
			pNextNode = static_cast<_MyData *>(m_pBase->GetBucket(m_iBucket));
		}

		// This check is useless, but Turbine does it. Probably inlined.
		if (m_pNodePtr)
			delete m_pNodePtr;

		while (!pNextNode)
		{
			// Dead node. Use the next bucket.
			m_iBucket++;
			m_pLast = NULL;

			if (m_iBucket>= m_pBase->GetBucketCount())
			{
				// The end has been reached.
				m_bEndReached = TRUE;
				m_pNodePtr = NULL;
				return;
			}

			pNextNode = static_cast<_MyData *>(m_pBase->GetBucket(m_iBucket));
		}

		m_pNodePtr = pNextNode;
	}

	// -- Pea you made this one up. It was probably inlined. --
	_MyData *GetCurrent()
	{
		return m_pNodePtr;
	}

	// -- Pea you made this one up. It was probably inlined. --
	BOOL EndReached()
	{
		return m_bEndReached;
	}

private:

	_MyHash *m_pBase; // 0x00
	size_t m_iBucket; // 0x04
	_MyData *m_pNodePtr; // 0x08
	_MyData *m_pLast; // 0x0C
	BOOL m_bEndReached; // 0x10
};

template<class _Mty>
class LongNIValHash : public HashBase<uint32_t>
{
public:
	typedef HashBase<uint32_t> _Mybase;
	typedef HashBaseData<uint32_t> _MybaseData;
	typedef HashBaseIter<uint32_t> _MybaseIter;

	typedef LongNIValHashData<_Mty> _MyData;
	typedef LongNIValHashIter<_Mty> _MyIter;

	LongNIValHash(size_t buckets = 16)
		: _Mybase(buckets)
	{
	}

	virtual ~LongNIValHash()
	{
		destroy_contents();
	}

	void add(_Mty Data, uint32_t Key)
	{
		_MyData *pData = new _MyData(Data, Key);
		_Mybase::add(pData);
	}

	_MybaseData *remove(uint32_t Key)
	{
		return _Mybase::remove(Key);
	}

	BOOL remove(uint32_t Key, _Mty *pOutData)
	{
		// We are converting the data pointers to their parent objects.
		_MybaseData *pData = _Mybase::remove(Key);

		if (!pData)
			return FALSE;

		_MyData *pInternal = static_cast<_MyData*>(pData);
		*pOutData = pInternal->m_Data;

		delete pInternal;

		return TRUE;
	}

	BOOL lookup(uint32_t Key, _Mty *pOutData)
	{
		// We are converting the data pointers to their parent objects.
		_MybaseData *pData = _Mybase::lookup(Key);

		if (!pData)
			return FALSE;

		_MyData *pInternal = static_cast<_MyData*>(pData);
		*pOutData = pInternal->m_Data;

		return TRUE;
	}

	void destroy_contents()
	{
		_MyIter it(this);

		while (!it.EndReached())
			it.DeleteCurrent();
	}

	BOOL clobber(_Mty *data, uint32_t dataKey)
	{
		// !! modifies data

		_MyData *pNewData = new _MyData(*data, dataKey);

		_MybaseData *pOldData = _Mybase::clobber(pNewData);
		if (!pOldData)
			return FALSE;

		_MyData *_oldData = static_cast<_MyData*>(pOldData);

		*data = _oldData->m_Data;
		delete _oldData;

		return TRUE;
	}
};

// TODO: LongNIHash

template<class _Mty>
class LongNIHashData
{
public:
	typedef uint32_t _Kty;

	LongNIHashData()
	{
		next = NULL;
		data = NULL;
		key = 0;
	}

	// added this for safe cleanup
	virtual ~LongNIHashData() {
	}

	LongNIHashData<_Mty> *next;
	_Mty *data;
	_Kty key;
};

template<typename _Mty>
class LongNIHashIter
{
public:
	typedef LongNIHash<_Mty> _MyHash;
	typedef LongNIHashData<_Mty> _MyData;
	typedef LongNIHashIter<_Mty> _MyIter;

	LongNIHashIter(_MyHash *pBase)
	{
		m_pBase = pBase;
		m_iBucket = -1;
		m_pNodePtr = NULL;
		m_bEndReached = FALSE;

		if (m_pBase)
		{
			do
			{
				if ((++m_iBucket) <m_pBase->GetBucketCount())
				{
					m_pNodePtr = m_pBase->GetBucket(m_iBucket);
				}
				else
				{
					m_bEndReached = TRUE;
					break;
				}
			} while (!m_pNodePtr);
		}
		else
			m_bEndReached = TRUE;
	}

	void Next()
	{
		if (m_bEndReached)
			return;

		m_pNodePtr = m_pNodePtr->next;
		assert(!m_pNodePtr || !IsBadReadPtr(m_pNodePtr, 4));

		while (!m_pNodePtr)
		{
			if ((++m_iBucket) <m_pBase->GetBucketCount())
			{
				m_pNodePtr = m_pBase->GetBucket(m_iBucket);
			}
			else
			{
				m_bEndReached = TRUE;
				break;
			}
		}
	}

	// -- Pea you made this one up. It was probably inlined. --
	_MyData *GetCurrent()
	{
		return m_pNodePtr;
	}

	// -- Pea you made this one up. It was probably inlined. --
	_Mty *GetCurrentData()
	{
		return (m_pNodePtr ? m_pNodePtr->data : NULL);
	}

	// -- Pea you made this one up. It was probably inlined. --
	BOOL EndReached()
	{
		return m_bEndReached;
	}

private:

	_MyHash *m_pBase; // 0x00
	size_t m_iBucket; // 0x04
	_MyData *m_pNodePtr; // 0x08
	BOOL m_bEndReached; // 0x0C
};

template<typename _Mty>
class LongNIHash
{
public:
	typedef uint32_t _Kty;
	typedef LongNIHashData<_Mty> _MyData;
	typedef LongNIHashIter<_Mty> _MyIter;

	LongNIHash(size_t buckets)
	{
		hash_table = NULL;
		bucket_count = buckets;

		InternalInit(buckets);
	}

	virtual ~LongNIHash()
	{
		DeleteAll();

		delete[] hash_table;
		hash_table = NULL;
	}

	void InternalInit(size_t buckets)
	{
		hash_table = new _MyData *[bucket_count];

		// ZeroMemory(hash_table, sizeof(_MyData *)  *bucket_count);
		memset(hash_table, 0, sizeof(_MyData *)  *bucket_count);
	}

	void DeleteAll()
	{
		_MyIter it(this);

		while (!it.EndReached())
		{
			_Mty *pNodeData = it.GetCurrentData();

			try
			{
				it.Next();

				if (pNodeData)
					delete pNodeData;
			}
			catch (...)
			{
				SERVER_ERROR << "Error in HashData DeleteAll";
			}
		}

		flush();
	}

	void flush()
	{
		for (uint32_t i = 0; i <bucket_count; i++)
		{
			_MyData *pData = hash_table[i];

			while (pData)
			{
				_MyData *pNextData = pData->next; // Next entry in chain.
				delete pData;

				pData = pNextData;
			}

			hash_table[i] = NULL;
		}
	}

	_Mty *remove(_Kty Key)
	{
		size_t BucketIndex = (Key ^ (Key>> 16)) % bucket_count;
		_MyData **BucketOffset = &hash_table[BucketIndex];

		_MyData *RootEntry = BucketOffset[0];

		if (!RootEntry)
			return NULL;

		if (RootEntry->key == Key)
		{
			// First entry of bucket was a match.
			hash_table[BucketIndex] = RootEntry->next;
			_Mty *pData = RootEntry->data;
			delete RootEntry;

			return pData;
		}

		_MyData *CurrentEntry = RootEntry;

		// Iterate through the chain in this bucket.
		while (CurrentEntry->next)
		{
			_MyData *NextEntry = CurrentEntry->next;

			if (NextEntry->key == Key)
			{
				// Next entry is the one we want, so remove from chain.

				// This is a dumb check, but for whatever reason the client compiled this way.
				if (CurrentEntry)
				{
					CurrentEntry->next = NextEntry->next;
					_Mty *pData = NextEntry->data;
					delete NextEntry;

					return pData;
				}
				else
				{
					hash_table[BucketIndex] = NextEntry->next;
					_Mty *pData = NextEntry->data;
					delete NextEntry;

					return pData;
				}
			}

			CurrentEntry = NextEntry;
		}

		// No match found.
		return NULL;
	}

	BOOL add(_Mty *pData, _Kty Key)
	{
		size_t BucketIndex = (Key ^ (Key>> 16)) % bucket_count;
		_MyData **BucketOffset = &hash_table[BucketIndex];

		_MyData *pHashData = new _MyData;

		if (!pHashData)
			return FALSE;

		pHashData->key = Key;
		pHashData->data = pData;
		pHashData->next = *BucketOffset;
		*BucketOffset = pHashData;

		return TRUE;
	}

	_Mty *lookup(_Kty Key)
	{
		_MyData *pEntry = hash_table[(Key ^ (Key>> 16)) % bucket_count];

		while (pEntry)
		{
			if (pEntry->key == Key)
				return pEntry->data;

			pEntry = pEntry->next;
		}

		return NULL;
	}

	// -- Pea you made this one up. It was probably inlined. --
	_MyData *GetTable()
	{
		return hash_table;
	}

	// -- Pea you made this one up. It was probably inlined. --
	size_t GetBucketIndex(_Kty Key)
	{
		return((Key ^ (Key>> 16)) % bucket_count);
	}

	// -- Pea you made this one up. It was probably inlined. --
	_MyData *GetBucket(size_t index)
	{
		return hash_table[index];
	}

	// -- Pea you made this one up. It was probably inlined. --
	size_t GetBucketCount()
	{
		return bucket_count;
	}

	// -- Pea you made this one up. It was probably inlined. --
	void SetBucket(size_t index, _MyData *pData)
	{
		hash_table[index] = pData;
	}

private:
	_MyData **hash_table; // 0x00
	size_t bucket_count; // 0x04
};


/*
template<typename _Kty, class _Dty>
class PackableHashTable
{
public:
	PackableHashTable()
	{
		// ...
	}

	virtual ~PackableHashTable()
	{
		Destroy();
	}

	void Destroy()
	{
		// ...
	}

	void EmptyContents()
	{
		// ...
	}

	virtual BOOL UnPack(BYTE **ppData, ULONG iSize)
	{
		// ...
		return TRUE;
	}
};
*/









