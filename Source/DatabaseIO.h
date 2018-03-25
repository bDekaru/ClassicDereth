
#pragma once


struct AccountInformation_t
{
	unsigned int id;
	std::string username;
	unsigned int dateCreated;
	unsigned int access;
};

struct CharacterDesc_t
{
	unsigned int account_id;
	unsigned int weenie_id;
	std::string name;
	unsigned int date_created;
	unsigned short instance_ts;
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
	DBIO_GLOBAL_HOUSING_DATA = 3
};

class CDatabaseIO
{
public:
	CDatabaseIO();
	virtual ~CDatabaseIO();

	const unsigned int BLOB_QUERY_BUFFER_LENGTH = 5000000;
	void *blob_query_buffer = NULL;

	const unsigned int MIN_USERNAME_LENGTH = 1;
	const unsigned int MAX_USERNAME_LENGTH = 32;
	const unsigned int MIN_PASSWORD_LENGTH = 1;
	const unsigned int MAX_PASSWORD_LENGTH = 64;
	bool VerifyAccount(const char *username, const char *password, AccountInformation_t *pAccountInfo, int *pError);
	bool CreateAccount(const char *username, const char *password, int *pError, const char *ipaddress);

	std::list<CharacterDesc_t> GetCharacterList(unsigned int accountid);
	bool CreateCharacter(unsigned int account_id, unsigned int weenie_id, const char *name);
	bool DeleteCharacter(unsigned int weenie_id);
	bool SetCharacterInstanceTS(unsigned int weenie_id, unsigned int instance_ts);

	std::list<unsigned int> GetWeeniesAt(unsigned int block_id);
	bool AddOrUpdateWeenieToBlock(unsigned int weenie_id, unsigned int block_id);
	bool RemoveWeenieFromBlock(unsigned int weenie_id);

	unsigned int GetHighestWeenieID(unsigned int min_range, unsigned int max_range);
	bool IsCharacterNameOpen(const char *name);
	bool IsPlayerCharacter(unsigned int weenie_id);
	DWORD GetPlayerCharacterId(const char *name);
	std::string GetPlayerCharacterName(DWORD weenie_id);

	bool CreateOrUpdateWeenie(unsigned int weenie_id, unsigned int top_level_object_id, unsigned int block_id, void *data, unsigned int data_length);
	bool GetWeenie(unsigned int weenie_id, unsigned int *top_level_object_id, unsigned int *block_id, void **data, unsigned long *data_length);
	bool DeleteWeenie(unsigned int weenie_id);
	bool IsWeenieInDatabase(unsigned int weenie_id);

	bool GetGlobalData(DBIOGlobalDataID id, void **data, unsigned long *data_length);
	bool CreateOrUpdateGlobalData(DBIOGlobalDataID id, void *data, unsigned long data_length);

	CLockable _pendingSavesLock;
	std::unordered_map<DWORD, DWORD> _pendingSaves;

	void IncrementPendingSave(DWORD weenie_id);
	void DecrementPendingSave(DWORD weenie_id);
	DWORD GetNumPendingSaves(DWORD weenie_id);
};


