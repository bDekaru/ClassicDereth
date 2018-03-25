
#pragma once

class ServerCellManager
{
public:
	ServerCellManager();
	virtual ~ServerCellManager();

	class CObjCell *GetObjCell(DWORD cell_id);
};

