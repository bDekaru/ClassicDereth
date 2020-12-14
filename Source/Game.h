#ifndef GAME_H
#define GAME_H

#include "WeenieObject.h"

class GameWeenie : public CWeenieObject
{
public:
	GameWeenie* AsGame() override { return this; }

	void PostSpawn() override;
	void NotifyRemoveFromWorld() override;
};

#endif // GAME_H
