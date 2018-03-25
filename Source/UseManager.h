
#pragma once

class CUseEventData
{
public:
	CUseEventData();
	
	virtual void SetupUse();
	void Begin();

	void CheckTimeout();
	void CancelMoveTo();

	virtual void OnMotionDone(DWORD motion, BOOL success);
	virtual void OnUseAnimSuccess(DWORD motion);

	void ExecuteUseAnimation(DWORD motion, MovementParameters *params = NULL);
	double DistanceToTarget();
	double HeadingDifferenceToTarget();
	bool InUseRange();
	CWeenieObject *GetTarget();
	CWeenieObject *GetTool();
	void MoveToUse();

	virtual void HandleMoveToDone(DWORD error);
	virtual void OnReadyToUse() = 0;

	virtual void Update();
	virtual void Cancel(DWORD error = 0);
	virtual void Done(DWORD error = 0);

	virtual bool IsInventoryEvent() { return false; }

	class UseManager *_manager = NULL;
	class CWeenieObject *_weenie = NULL;

	DWORD _target_id = 0;
	DWORD _tool_id = 0; // when using one item on another item
	bool _move_to = false;
	double _max_use_distance = FLT_MAX;
	double _timeout = FLT_MAX;
	DWORD _active_use_anim = 0;
};

class CGenericUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;
	void Finish();

	DWORD _do_use_animation = 0;
	bool _do_use_message = true;
	bool _do_use_emote = true;
	bool _do_use_response = true;
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
	virtual void OnUseAnimSuccess(DWORD motion) override;

	DWORD _source_item_id = 0;
	DWORD _target_container_id = 0;
	DWORD _target_slot = 0;
};

class CDropInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;
};

class CMoveToWieldInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;

	DWORD _sourceItemId = 0;
	DWORD _targetLoc = 0;
};

class CStackMergeInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;

	DWORD _sourceItemId = 0;
	DWORD _targetItemId = 0;
	DWORD _amountToTransfer = 0;
};

class CStackSplitToContainerInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;

	DWORD _sourceItemId = 0;
	DWORD _targetContainerId = 0;
	DWORD _targetSlot = 0;
	DWORD _amountToTransfer = 0;
};

class CStackSplitTo3DInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;

	DWORD _sourceItemId = 0;
	DWORD _amountToTransfer = 0;
};

class CStackSplitToWieldInventoryUseEvent : public CInventoryUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;

	DWORD _sourceItemId = 0;
	DWORD _targetLoc = 0;
	DWORD _amountToTransfer = 0;
};


class UseManager
{
public:
	UseManager(class CWeenieObject *weenie);
	~UseManager();

	void Update();
	void Cancel();

	void BeginUse(CUseEventData *data);

	void OnUseCancelled(DWORD error = 0);
	void OnUseDone(DWORD error = 0);

	void OnDeath(DWORD killer_id);
	void HandleMoveToDone(DWORD error);
	void OnMotionDone(DWORD motion, BOOL success);

	bool IsUsing();

	void MarkForCleanup(CUseEventData *data);

private:
	class CWeenieObject *_weenie = NULL;

	double _next_allowed_use = 0.0;
	CUseEventData *_useData = NULL;
	CUseEventData *_cleanupData = NULL;
};