
#pragma once

class CUseEventData
{
public:
	CUseEventData();
	virtual ~CUseEventData() = default;
	
	virtual void SetupUse();
	void Begin();

	void CheckTimeout();
	void CancelMoveTo();

	virtual void OnMotionDone(uint32_t motion, BOOL success);
	virtual void OnUseAnimSuccess(uint32_t motion);

	void ExecuteUseAnimation(uint32_t motion, MovementParameters *params = NULL);
	double DistanceToTarget();
	double HeadingDifferenceToTarget();
	bool InUseRange();
	bool InMoveRange();
	CWeenieObject *GetTarget();
	CWeenieObject *GetTool();
	CWeenieObject *GetSourceItem();
	void MoveToUse();
	void SetupRecall();

	virtual void HandleMoveToDone(uint32_t error);
	virtual void OnReadyToUse() = 0;

	virtual void Update();
	virtual void Cancel(uint32_t error = 0);
	virtual void Done(uint32_t error = 0);

	virtual bool IsInventoryEvent() { return false; }

	class UseManager *_manager = NULL;
	class CWeenieObject *_weenie = NULL;

	uint32_t _target_id = 0;
	uint32_t _tool_id = 0; // when using one item on another item
	bool _move_to = false;
	double _max_use_distance = FLT_MAX;
	double _timeout = FLT_MAX;
	uint32_t _active_use_anim = 0;
	Position _initial_use_position;
	uint32_t _source_item_id = 0;
	bool _give_event = false;
	bool _recall_event = false;

	bool QuestRestrictions(CWeenieObject *target);
};

class CGenericUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
	void Finish();

	uint32_t _do_use_animation = 0;
	bool _do_use_message = true;
	bool _do_use_emote = true;
	bool _do_use_response = true;
};

class callback_use_event : public CGenericUseEvent
{
public:
	using callback_fn_t = std::function<void(uint32_t)>;
	using setup_fn_t = std::function<void(CUseEventData*)>;

	callback_use_event(setup_fn_t setup, callback_fn_t callback):
		m_setup(setup), m_callback(callback) {}

	virtual void OnReadyToUse() override
	{
		m_setup(this);
	}

	virtual void OnUseAnimSuccess(uint32_t motion) override
	{
		m_callback(motion);
	}

protected:
	setup_fn_t m_setup;
	callback_fn_t m_callback;
};

class motion_use_event : public callback_use_event
{
public:

	motion_use_event(uint32_t motion, callback_fn_t callback) :
		callback_use_event(setup, callback)
	{
		_do_use_animation = motion;
	}

	// this is poorly named, determines whether or not to call UseDone
	virtual bool IsInventoryEvent() override { return true; }

private:
	static void setup(CUseEventData* useEvent)
	{
		// needs GR+ (rtti?) for dynamic_cast, use reinterpret
		//CGenericUseEvent *ued = dynamic_cast<CGenericUseEvent*>(useEvent);
		CGenericUseEvent *ued = reinterpret_cast<CGenericUseEvent*>(useEvent);
		ued->ExecuteUseAnimation(ued->_do_use_animation);
	}
};

class CRecallUseEvent : public CUseEventData
{
public:
	// this is poorly named, determines whether or not to call UseDone
	virtual bool IsInventoryEvent() override { return true; }
};


class CActivationUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
};

class CInventoryUseEvent : public CUseEventData
{
public:
	virtual void SetupUse() override;

	virtual bool IsInventoryEvent() override { return true; }
};

class CPickupInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;

	uint32_t _source_item_id = 0;
	uint32_t _target_container_id = 0;
	uint32_t _target_slot = 0;
};

class CDropInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
};

class CMoveToWieldInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;

	uint32_t _sourceItemId = 0;
	uint32_t _targetLoc = 0;
};

class CStackMergeInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;

	uint32_t _sourceItemId = 0;
	uint32_t _targetItemId = 0;
	uint32_t _amountToTransfer = 0;
};

class CStackSplitToContainerInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;

	uint32_t _sourceItemId = 0;
	uint32_t _targetContainerId = 0;
	uint32_t _targetSlot = 0;
	uint32_t _amountToTransfer = 0;
};

class CStackSplitTo3DInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;

	uint32_t _sourceItemId = 0;
	uint32_t _amountToTransfer = 0;
};

class CStackSplitToWieldInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;

	uint32_t _sourceItemId = 0;
	uint32_t _targetLoc = 0;
	uint32_t _amountToTransfer = 0;
};

class CGiveEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
	virtual void Cancel(uint32_t error = 0) override;

	virtual bool IsInventoryEvent() override { return true; }

	uint32_t _transfer_amount = 0;
};

class CTradeUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
};

class UseManager
{
public:
	UseManager(class CWeenieObject *weenie);
	~UseManager();

	void Update();
	void Cancel();

	void BeginUse(CUseEventData *data);

	void OnUseCancelled(uint32_t error = 0);
	void OnUseDone(uint32_t error = 0);

	void OnDeath(uint32_t killer_id);
	void HandleMoveToDone(uint32_t error);
	void OnMotionDone(uint32_t motion, BOOL success);

	bool IsUsing();
	bool IsMoving();

	void MarkForCleanup(CUseEventData *data);
	double _next_allowed_use = Timer::cur_time;

private:
	class CWeenieObject *_weenie = NULL;
		
	CUseEventData *_useData = NULL;
	CUseEventData *_cleanupData = NULL;
};
