
#pragma once

class AnimationPackage
{
	friend class TurbineAnimation;
public:
	AnimationPackage( WORD wStance, WORD wIndex, float fSpeed );

	virtual bool Initialize( );

	inline uint32_t GetBaseFrame( ) {
		return ((m_fSpeed >= 0)? m_dwStartFrame : m_dwEndFrame );
	}

private:
	WORD	m_wStance;
	WORD	m_wIndex;
	float	m_fSpeed;

	double	m_fStartTime;
	uint32_t	m_dwCurrentFrame;
	uint32_t	m_dwStartFrame;
	uint32_t	m_dwEndFrame;

	uint32_t	m_dwTarget;
	uint32_t	m_dwAction;	//For identifying actions (lifestone recall, etc.)
	uint32_t	m_dwActionData[10];
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