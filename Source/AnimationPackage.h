
#pragma once

class AnimationPackage
{
	friend class TurbineAnimation;
public:
	AnimationPackage( WORD wStance, WORD wIndex, float fSpeed );

	virtual bool Initialize( );

	inline DWORD GetBaseFrame( ) {
		return ((m_fSpeed >= 0)? m_dwStartFrame : m_dwEndFrame );
	}

private:
	WORD	m_wStance;
	WORD	m_wIndex;
	float	m_fSpeed;

	double	m_fStartTime;
	DWORD	m_dwCurrentFrame;
	DWORD	m_dwStartFrame;
	DWORD	m_dwEndFrame;

	DWORD	m_dwTarget;
	DWORD	m_dwAction;	//For identifying actions (lifestone recall, etc.)
	DWORD	m_dwActionData[10];
};

class SequencedAnimation : public AnimationPackage
{
public:
	SequencedAnimation( WORD wSequence, WORD wStance, WORD wIndex, float fSpeed );
	
	inline WORD	GetSequence( ) {
		return m_wSequence;
	}
private:
	WORD	m_wSequence;
};