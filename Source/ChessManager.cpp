#include <StdAfx.h>
#include "ChessManager.h"
#include "Player.h"
#include "Game.h"

namespace GDLE::Chess
{

ChessMatch* GameInformation::NewMatch()
{
	assert(m_match == nullptr);
	m_match = new ChessMatch(m_weenie);
	return m_match;
}

void ChessManager::RegisterGameBoard(GameWeenie* game)
{
	assert(m_games.find(game->GetID()) == m_games.end());
	m_games.emplace(game->GetID(), game);
}

void ChessManager::UnregisterGameBoard(GameWeenie* game)
{
	GameStore::iterator const itr = m_games.find(game->GetID());
	assert(itr != m_games.end());

	// a landblock with an active match on it should never be unloaded
	assert(!itr->second.GetMatch());

	m_games.erase(itr);
}

void ChessManager::Update()
{
	for (GameStore::value_type& pair : m_games)
	{
		ChessMatch* match = pair.second.GetMatch();
		if (!match)
			continue;

		match->Update();

		if (match->GetState() == ChessStateFinished)
			pair.second.DeleteMatch();
	}
}

void ChessManager::Join(CPlayerWeenie* player, CWeenieObject* object)
{
	GameWeenie* game = object->AsGame();
	if (!game)
	{
		ChessMatch::SendJoinGameResponse(player, object->GetID(), std::nullopt);
		return;
	}

	if (GetGame(player->GetID()))
	{
		ChessMatch::SendJoinGameResponse(player, game->GetID(), std::nullopt);
		return;
	}

	GameStore::iterator const itr = m_games.find(game->GetID());
	assert(itr != m_games.end());

	ChessMatch* match = itr->second.GetMatch();
	if (!match)
		match = itr->second.NewMatch();

	match->Join(player);
}

void ChessManager::Move(CPlayerWeenie* player, ChessPieceCoord const& from, ChessPieceCoord const& to) const
{
	if (ChessMatch* match = GetGame(player->GetID()))
		match->MoveEnqueue(player, from, to);
}

void ChessManager::MovePass(CPlayerWeenie* player) const
{
	if (ChessMatch* match = GetGame(player->GetID()))
		match->MovePassEnqueue(player);
}

void ChessManager::Quit(CPlayerWeenie* player) const
{
	if (ChessMatch* match = GetGame(player->GetID()))
		match->QuitEnqueue(player);
}

void ChessManager::Stalemate(CPlayerWeenie* player, bool const on) const
{
	if (ChessMatch* match = GetGame(player->GetID()))
		match->StalemateEnqueue(player, on);
}

void ChessManager::ChallengeAi(CPlayerWeenie* player) const
{
	if (ChessMatch* match = GetGame(player->GetID()))
		match->AddAi();
}

void ChessManager::PieceReady(uint32_t const gameGuid, uint32_t const pieceGuid)
{
	GameStore::iterator const itr = m_games.find(gameGuid);
	assert(itr != m_games.end());

	ChessMatch* match = itr->second.GetMatch();
	assert(match);

	match->PieceReady(pieceGuid);
}

ChessMatch* ChessManager::GetGame(uint32_t const guid) const
{
	for (GameStore::value_type const& pair : m_games)
		if (ChessMatch* match = pair.second.GetMatch())
			if (match->IsInMatch(guid))
				return match;
	return nullptr;
}

} // GDLE::Chess
