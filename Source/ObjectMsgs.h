
#pragma once

class BinaryWriter;
class CWeenieObject;
class CMonsterWeenie;
class CPlayerWeenie;

extern BinaryWriter *CreateObject(CWeenieObject *pEntity);	//0x0000F745
extern BinaryWriter *UpdateObject(CWeenieObject *pEntity);
extern BinaryWriter *IdentifyObjectFail(CWeenieObject *pEntity, bool bShowLevel);	//0x000000C9
extern BinaryWriter *IdentifyObject(CWeenieObject *pSource, CWeenieObject *pEntity, uint32_t overrideId = 0);	//0x000000C9
extern BinaryWriter *LoginCharacter(CPlayerWeenie *pPlayer);	//0x00000013
extern BinaryWriter *HealthUpdate(CWeenieObject *pWeenie);	//0x000001C0
extern BinaryWriter *ItemManaUpdate(CWeenieObject *pWeenie);		//0x00000264
extern BinaryWriter *InventoryEquip(uint32_t dwItemID, uint32_t dwCoverage); //0x00000023
extern BinaryWriter *InventoryMove(uint32_t dwItemID, uint32_t dwContainerID, uint32_t dwSlot, uint32_t dwType); //0x00000022
extern BinaryWriter *InventoryDrop(uint32_t dwItemID); //0x0000019A
extern BinaryWriter *MoveUpdate(CWeenieObject *pEntity);		//0x0000F748

