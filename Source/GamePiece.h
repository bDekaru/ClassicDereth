#ifndef GAME_PIECE_H
#define GAME_PIECE_H

#include "WeenieObject.h"
#include "Monster.h"
#include "Frame.h"

enum GamePieceState
{
	GamePieceStateNone,
	GamePieceStateMoveToSquare,
	GamePieceStateWaitingForMoveToSquare,
	GamePieceStateMoveToAttack,
	GamePieceStateWaitingForMoveToAttack,
	GamePieceStateCombat
};

class GamePieceWeenie : public CMonsterWeenie
{
public:
	GamePieceWeenie()
		: m_guid(), m_state(), m_victim() { }

	GamePieceWeenie* AsGamePiece() override { return this; }

	void SetGuid(uint32_t const guid) { m_guid = guid; }

	void MoveEnqueue(Position const& to);
	void AttackEnqueue(Position const& to, uint32_t victim);

	void Tick() override;

private:
	void MoveWeenie(Position const& to, float distanceToObject, bool finalHeading);

	void HandleMoveToDone(uint32_t error) override;
	void OnDealtDamage(DamageEventData& damageData) override;

	uint32_t m_guid;
	GamePieceState m_state;
	uint32_t m_victim;
	Position m_position;
};

#endif // GAME_PIECE_H
