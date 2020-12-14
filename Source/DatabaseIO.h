
#pragma once

struct AccountInformation_t
{
	unsigned int id;
	std::string username;
	unsigned int dateCreated;
	unsigned int access;
	bool banned;
};

struct CharacterDesc_t
{
	unsigned int account_id;
	unsigned int weenie_id;
	std::string name;
	unsigned int date_created;
	unsigned short instance_ts;
	uint32_t ts_deleted;
	double gag_timer;
};

struct CharacterGags_t
{
	std::string name;
	double gag_timer;
	uint32_t gag_count;
};

struct HouseInfo_t
{
	unsigned int house_id;
	//std::string houseBlob;
	uint8_t *houseBlob;
	size_t houseBlobLen;
};

struct CharacterSquelch_t
{
	unsigned int squelched_id = 0;
	unsigned int account_id = 0;
	bool isIp = false;
	bool isSpeech = false;
	bool isTell = false;
	bool isCombat = false;
	bool isMagic = false;
	bool isEmote = false;
	bool isAdvancement = false;
	bool isAppraisal = false;
	bool isSpellcasting = false;
	bool isAllegiance = false;
	bool isFellowship = false;
	bool isCombatEnemy = false;
	bool isRecall = false;
	bool isCrafting = false;

public:
	

};

enum DBIOError
{
	DBIO_ERROR_NONE = 0,
	CREATEACCOUNT_ERROR_QUERY_FAILED,
	CREATEACCOUNT_ERROR_BAD_USERNAME,
	CREATEACCOUNT_ERROR_BAD_PASSWORD,
	VERIFYACCOUNT_ERROR_QUERY_FAILED,
	VERIFYACCOUNT_ERROR_DOESNT_EXIST,
	VERIFYACCOUNT_ERROR_BAD_LOGIN
};

enum DBIOGlobalDataID
{
	DBIO_GLOBAL_ALLEGIANCE_DATA = 1,
	DBIO_GLOBAL_BAN_DATA = 2,
	DBIO_GLOBAL_HOUSING_DATA = 3,
	DBIO_GLOBAL_ID_RANGE_START = 4
};

class CDatabaseIO
{
public:
	CDatabaseIO();
	virtual ~CDatabaseIO();

	const unsigned int BLOB_QUERY_BUFFER_LENGTH = 5000000;
	uint8_t *blob_query_buffer = NULL;

	const unsigned int MIN_USERNAME_LENGTH = 1;
	const unsigned int MAX_USERNAME_LENGTH = 32;
	const unsigned int MIN_PASSWORD_LENGTH = 1;
	const unsigned int MAX_PASSWORD_LENGTH = 64;

	bool IsUserNameValid(const std::string &username);
	bool VerifyAccount(const char *username, const char *password, AccountInformation_t *pAccountInfo, int *pError);
	bool VerifyAccount(void* dbc, const char *username, const char *password, AccountInformation_t *pAccountInfo, int *pError);
	bool CreateAccount(const char *username, const char *password, int *pError, const char *ipaddress);
	bool CreateAccount(void* dbc, const char *username, const char *password, int *pError, const char *ipaddress);

	std::list<CharacterDesc_t> GetCharacterList(unsigned int accountid);
	CharacterDesc_t GetCharacterInfo(unsigned int weenie_id);

	std::list<CharacterSquelch_t> GetCharacterSquelch(unsigned int character_id);
	bool SaveCharacterSquelch(unsigned int character_id, CharacterSquelch_t data);
	bool RemoveCharacterSquelch(unsigned int character_id, CharacterSquelch_t data);

	std::list<CharacterGags_t> GetServerGagData();
	CharacterGags_t GetCharacterGagData(unsigned int character_id);
	bool SaveServerGag(unsigned int character_id, double duration, bool account = false);
	bool RemoveServerGag(unsigned int character_id, bool account = false);
	


	bool CreateCharacter(unsigned int account_id, unsigned int weenie_id, const char *name);
	bool DeleteCharacter(unsigned int weenie_id, bool force = false);
	bool IsCharacterDeleting(unsigned int weenie_id);
	bool RestoreCharacter(unsigned int weenie_id);
	bool SetCharacterInstanceTS(unsigned int weenie_id, unsigned int instance_ts);
	
	bool UpdateBan(unsigned int account_id, bool ban);

	std::list<unsigned int> GetWeeniesAt(unsigned int block_id);
	bool AddOrUpdateWeenieToBlock(unsigned int weenie_id, unsigned int block_id);
	bool RemoveWeenieFromBlock(unsigned int weenie_id);
	bool IDRangeTableExistsAndValid();
	std::list<unsigned int> GetNextIDRange(unsigned int rangeStart, unsigned int count);
	unsigned int GetHighestWeenieID(unsigned int min_range, unsigned int max_range);

	bool IsCharacterNameOpen(const char *name);
	bool IsPlayerCharacter(unsigned int weenie_id);
	uint32_t GetPlayerCharacterId(const char *name);
	std::string GetPlayerCharacterName(uint32_t weenie_id);
	std::list<HouseInfo_t> GetHousingData();
	uint32_t GetPlayerAccountId(unsigned int character_id);

	bool CreateOrUpdateWeenie(unsigned int weenie_id, unsigned int top_level_object_id, unsigned int block_id, void *data, unsigned int data_length);
	bool GetWeenie(unsigned int weenie_id, unsigned int *top_level_object_id, unsigned int *block_id, void **data, unsigned long *data_length);
	bool DeleteWeenie(unsigned int weenie_id);
	bool IsWeenieInDatabase(unsigned int weenie_id);

	bool GetGlobalData(DBIOGlobalDataID id, void **data, unsigned long *data_length);
	bool CreateOrUpdateGlobalData(DBIOGlobalDataID id, void *data, unsigned long data_length);

	bool CreateOrUpdateHouseData(unsigned int house_id, void *data, unsigned int data_length);
	bool GetHouseData(unsigned int house_id, void **data, unsigned long *data_length);

	CLockable _pendingSavesLock;
	std::unordered_map<uint32_t, uint32_t> _pendingSaves;

	void IncrementPendingSave(uint32_t weenie_id);
	void DecrementPendingSave(uint32_t weenie_id);
	uint32_t GetNumPendingSaves(uint32_t weenie_id);
	
	std::string LoadCharacterTitles(unsigned int character_id);
	bool SaveCharacterTitles(unsigned int character_id, std::string titles);
	bool DeleteCharacterTitles(unsigned int character_id);
	bool DeleteCharacterInventory(unsigned int character_id);

	std::string LoadCharacterCorpses(unsigned int character_id);
	bool SaveCharacterCorpses(unsigned int, std::string corpselist);

	std::string GetEmailOfAccount(unsigned int character_id);
	bool SetEmailOfAccount(std::string email, unsigned int character_id);
	bool IsEmailAlreadySet(unsigned int character_id);

	std::list<unsigned int> GetAllCharacterIDs();
	std::list<unsigned int> GetBannedCharacterIDs();
	std::list<unsigned int> GetBannedAccountIDs();
	bool IsAlreadyFriend(unsigned int character_id, unsigned int friend_id);
	bool AddCharacterFriend(unsigned int character_id, unsigned int friend_id);
	bool RemoveCharacterFriend(unsigned int character_id, unsigned int frenemy_id);
	std::list<CharacterDesc_t> GetFriends(unsigned int character_id);
	std::list<CharacterDesc_t> GetFriendsOf(unsigned int character_id);
	void ClearFriendsList(unsigned int character_id);
	void DeleteCharFriends(unsigned int character_id);

	bool SaveWindowData(unsigned int character_id, BYTE* data, unsigned long data_length);
	int LoadWindowData(unsigned int character_id, BYTE** data);

};


