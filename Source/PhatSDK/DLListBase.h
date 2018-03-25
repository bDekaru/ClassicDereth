
#pragma once

class DLListData
{
public:
	DLListData()
	{
		dllist_next = NULL;
		dllist_prev = NULL;
	}

	virtual ~DLListData() {
	}

	DLListData *dllist_next;
	DLListData *dllist_prev;
};

class DLListBase
{
public:

	DLListBase()
	{
		head_ = NULL;
		tail_ = NULL;
	}

	virtual ~DLListBase() {
	}

	void InsertAfter(DLListData *pNode, DLListData *pPlacement)
	{
		if (pPlacement)
		{
			pNode->dllist_next = pPlacement->dllist_next;

			if (pPlacement->dllist_next)
				pPlacement->dllist_next->dllist_prev = pNode;
			else
				tail_ = pNode;

			pPlacement->dllist_next = pNode;
			pNode->dllist_prev = pPlacement;
		}
		else
		{
			// Place at beginning.
			pNode->dllist_next = head_;

			if (head_)
				head_->dllist_prev = pNode;
			else
				tail_ = pNode;

			head_ = pNode;
			pNode->dllist_prev = NULL;
		}
	}

	void Remove(DLListData *pNode)
	{
		if (pNode->dllist_prev)
			pNode->dllist_prev->dllist_next = pNode->dllist_next;
		else
		{
			head_ = head_->dllist_next;

			if (head_)
				head_->dllist_prev = NULL;
		}

		if (pNode->dllist_next)
			pNode->dllist_next->dllist_prev = pNode->dllist_prev;
		else
		{
			tail_ = tail_->dllist_prev;

			if (tail_)
				tail_->dllist_next = NULL;
		}

		pNode->dllist_next = NULL;
		pNode->dllist_prev = NULL;
	}

	DLListData *GetFirstNode()
	{
		return head_;
	}

	DLListData *GetLastNode()
	{
		return tail_;
	}

	void RemoveAndDelete(DLListData *pNode)
	{
		Remove(pNode);
		delete pNode;
	}

	// Made up
	BOOL Empty()
	{
		return(!head_ ? TRUE : FALSE);
	}

	// Made up
	void DestroyContents()
	{
		DLListData *pNode;

		while (pNode = head_)
		{
			Remove(pNode);

			delete pNode;
		}
	}

	DLListData *head_;
	DLListData *tail_;
};

class LListData
{
public:
	LListData() {
		llist_next = NULL;
	}

	LListData *llist_next;
};

class LListBase
{
public:

	LListBase()
	{
		head_ = NULL;
		tail_ = NULL;
	}

	~LListBase() {    }

	// Made up
	DWORD Size()
	{
		LListData *pNode = head_;
		DWORD iCount = 0;

		while (pNode)
		{
			++iCount;
			pNode = pNode->llist_next;
		}

		return iCount;
	}

	// Made up
	void InsertAtEnd(LListData *pNode)
	{
		if (tail_)
		{
			tail_->llist_next = pNode;
			tail_ = pNode;
		}
		else
		{
			head_ = pNode;
			tail_ = pNode;
		}
	}

	// Made up
	LListData* RemoveHead()
	{
		LListData *pNode = head_;

		if (pNode)
		{
			head_ = pNode->llist_next;

			if (head_ == NULL)
				tail_ = NULL;

			pNode->llist_next = NULL;
		}

		return pNode;
	}

	// Made up
	BOOL Empty()
	{
		return(!head_ ? TRUE : FALSE);
	}

	// Made up
	void DestroyContents()
	{
		while (head_)
		{
			LListData *pNode = RemoveHead();
			delete pNode;
		}
	}

	LListData *head_;
	LListData *tail_;
};



