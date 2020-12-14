#include <StdAfx.h>
#include "GamePiece.h"
#include "World.h"
#include "MonsterAI.h"
#include "ChessManager.h"

void GamePieceWeenie::MoveEnqueue(Position const& to)
{
	m_state    = GamePieceStateMoveToSquare;
	m_position = to;
}

void GamePieceWeenie::AttackEnqueue(Position const& to, uint32_t const victim)
{
	m_state    = GamePieceStateMoveToAttack;
	m_position = to;
	m_victim   = victim;
}

void GamePieceWeenie::Tick()
{
	CMonsterWeenie::Tick();

	if (motions_pending())
		return;

	switch (m_state)
	{
		case GamePieceStateMoveToSquare:
		{
			MoveWeenie(m_position, 0.1f, true);
			m_state = GamePieceStateWaitingForMoveToSquare;
			break;
		}
		// visual awareness range of piece is only 1, make sure we are close enough to attack
		case GamePieceStateMoveToAttack:
		{
			MoveWeenie(m_position, 0.8f, false);
			m_state = GamePieceStateWaitingForMoveToAttack;
			break;
		}
		default:
			break;
	}
}

void GamePieceWeenie::HandleMoveToDone(uint32_t const error)
{
	CMonsterWeenie::HandleMoveToDone(error);

	switch (m_state)
	{
		// we are done, tell the match so the turn can finish
		case GamePieceStateWaitingForMoveToSquare:
		{
			sChessManager->PieceReady(m_guid, GetID());
			m_state = GamePieceStateNone;
			break;
		}
		// there is another piece on this square, attack it!
		case GamePieceStateWaitingForMoveToAttack:
		{
			CWeenieObject* victim = g_pWorld->FindObject(m_victim);
			assert(victim);
			m_MonsterAI->SetNewTarget(victim);

			m_state = GamePieceStateCombat;
			break;
		}
		default:
			break;
	}
}

void GamePieceWeenie::OnDealtDamage(DamageEventData& damageData)
{
	// weenie piece is dead, time to move into the square completly
	if (damageData.killingBlow)
		m_state = GamePieceStateMoveToSquare;
}

void GamePieceWeenie::MoveWeenie(Position const& to, float const distanceToObject, bool const finalHeading)
{
	MovementParameters params;
	params.distance_to_object = distanceToObject;
	params.use_spheres        = 0;
	params.use_final_heading  = finalHeading;
	params.desired_heading    = to.frame.get_heading();

	MovementStruct mvs;
	mvs.type   = MoveToPosition;
	mvs.pos    = to;
	mvs.params = &params;

	last_move_was_autonomous = false;
	movement_manager->PerformMovement(mvs);
}
