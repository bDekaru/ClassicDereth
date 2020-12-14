#pragma once

#include "BinaryReader.h"
//#include "Player.h"

class IClientMessage
{
public:
	virtual void Parse(BinaryReader *reader) = 0;
	virtual void Process() = 0;
};