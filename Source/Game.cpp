#include <StdAfx.h>
#include "Game.h"
#include "ChessManager.h"

void GameWeenie::PostSpawn()
{
	sChessManager->RegisterGameBoard(this);
}

void GameWeenie::NotifyRemoveFromWorld()
{
	sChessManager->UnregisterGameBoard(this);
}
