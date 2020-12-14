
#pragma once

class ServerCellManager
{
public:
	ServerCellManager();
	virtual ~ServerCellManager();

	class CObjCell *GetObjCell(uint32_t cell_id, bool bDoPostLoad = true);
};

