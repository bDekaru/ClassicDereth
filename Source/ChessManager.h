#ifndef CHESS_MANAGER_H
#define CHESS_MANAGER_H

#include "ChessMatch.h"

class CWeenieObject;
class CPlayerWeenie;

namespace GDLE::Chess
{

class GameInformation
{
public:
	explicit GameInformation(GameWeenie* weenie)
		: m_weenie(weenie), m_match(nullptr) { }

	// prevent copying
	//GameInformation(GameInformation&) = delete;
	GameInformation(const GameInformation&) = delete;
	GameInformation& operator=(GameInformation&) = delete;
	GameInformation(GameInformation&&) = delete;
	GameInformation& operator=(GameInformation&&) = delete;

	GameWeenie* GetWeenie() const { return m_weenie; }
	ChessMatch* GetMatch() const { return m_match; }

	ChessMatch* NewMatch();
	void DeleteMatch()
	{
		delete m_match;
		m_match = nullptr;
	}

private:
	GameWeenie* m_weenie;
	ChessMatch* m_match;
};

typedef std::unordered_map<uint32_t /*guid*/, GameInformation> GameStore;

class ChessManager
{
public:
	static ChessManager* Instance()
	{
		static ChessManager manager;
		return &manager;
	}

	// prevent copying
	ChessManager(ChessManager&) = delete;
	ChessManager operator= (ChessManager&) = delete;
	ChessManager(ChessManager&&) = delete;
	ChessManager operator= (ChessManager&&) = delete;

	void RegisterGameBoard(GameWeenie* game);
	void UnregisterGameBoard(GameWeenie* game);

	void Update();

	void Join(CPlayerWeenie* player, CWeenieObject* object);
	void Move(CPlayerWeenie* player, ChessPieceCoord const& from, ChessPieceCoord const& to) const;
	void MovePass(CPlayerWeenie* player) const;
	void Quit(CPlayerWeenie* player) const;
	void Stalemate(CPlayerWeenie* player, bool on) const;

	void ChallengeAi(CPlayerWeenie* player) const;

	void PieceReady(uint32_t gameGuid, uint32_t pieceGuid);

private:
	ChessManager() = default;

	ChessMatch* GetGame(uint32_t guid) const;

	GameStore m_games;
};

} // GDLE::Chess

#define sChessManager GDLE::Chess::ChessManager::Instance()

#endif // CHESS_MANAGER_H
