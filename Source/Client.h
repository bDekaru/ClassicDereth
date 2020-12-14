
#pragma once

#include "DatabaseIO.h"

class CClientEvents;
class CPacketController;
class BinaryWriter;
class BinaryReader;
typedef struct DungeonDesc_s DungeonDesc_t;

class CClient : public CKillable, public CLockable
{
	friend class CClientEvents;

public:
	CClient(SOCKADDR_IN *, WORD slot, struct AccountInformation_t &accountInfo);
	~CClient();

	void Think();
	void ThinkOutbound();
	void WorldThink();

	BOOL CheckAccount(const char *);
	BOOL CheckAddress(SOCKADDR_IN *);

	const char *GetAccount(); //The client's account name.
	const char *GetDescription(); //The status text to display for this client.

	void SetArrayPos(uint32_t arrayPos) { m_ArrayPos = arrayPos; }
	uint32_t GetArrayPos() { return m_ArrayPos; }

	WORD GetSlot(); // The client's assigned RecipientID.

	SOCKADDR_IN* GetHostAddress(); // The client's address.

	void SetLoginData(uint32_t dwUnixTime, uint32_t dwPortalStamp, uint32_t dwCellStamp);

	void IncomingBlob(BlobPacket_s *blob, double recvTime);
	void ProcessMessage(BYTE *data, uint32_t length, WORD);
	void SendNetMessage(BinaryWriter*, WORD group, BOOL game_event = 0, BOOL del = 1, bool ephemeral = false);
	void SendNetMessage(void *data, uint32_t length, WORD group, BOOL game_event = 0, BOOL del = 1, bool ephemeral = false);

	CPacketController *GetPacketController() { return m_pPC; }
	CClientEvents *GetEvents() { return m_pEvents; }

	int GetAccessLevel();
	void SetAccessLevel(unsigned int access); // does not save

	const AccountInformation_t &GetAccountInfo();
	const std::list<CharacterDesc_t> &GetCharacters();
	bool HasCharacter(uint32_t character_weenie_id);
	uint32_t IncCharacterInstanceTS(uint32_t character_weenie_id);

	bool RemoveCharacterGag(unsigned int character_id);
	bool IsCharacterGagged(unsigned int character_id);

private:
	void UpdateLoginScreen();

	// Non-world events.
	void EnterWorld();
	void ExitWorld();
	void DeleteCharacter(BinaryReader *);
	void RestoreCharacter(BinaryReader *);
	void CreateCharacter(BinaryReader *);
	void GenerateStarterGear(CWeenieObject *weenie, ACCharGenResult &cg, Sex_CG *scg);

	void SendLandblock(uint32_t dwFileID);
	void SendLandblockInfo(uint32_t dwFileID);
	void SendLandcell(uint32_t dwFileID);

	//
	BOOL CheckNameValidity(const char *name, int access, std::string &resultName);
	BOOL CheckBadName(const std::string name);

	// This is a dumbly separated way of parsing methods. Change this.
	CClientEvents *m_pEvents;

	// This handles parts of the network layer.
	CPacketController *m_pPC;

	struct CL_ClientVars_t
	{
		WORD slot; // Assigned RecipientID
		SOCKADDR_IN addr; // Socket address
		std::string account; // Account name

		// These are sent by the client
		uint32_t ClientLoginUnixTime = 0;
		uint32_t PortalStamp = 0;
		uint32_t CellStamp = 0;

		double fLoginTime = 0.0;

		bool bInitDats = false;
		bool bNeedChars = true;
		bool bInWorld = false;

	} m_vars;

	AccountInformation_t m_AccountInfo;
	std::list<CharacterDesc_t> m_Characters;
	std::list<CharacterDesc_t> m_CharactersSent;

	uint32_t m_ArrayPos = 0;
};



