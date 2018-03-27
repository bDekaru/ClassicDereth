
#include "StdAfx.h"
#include "Database2.h"
#include "DatabaseIO.h"
#include "SHA512.h"

DWORD64 g_RandomAdminPassword = 0; // this is used for the admin login

CDatabaseIO::CDatabaseIO()
{
	blob_query_buffer = new BYTE[BLOB_QUERY_BUFFER_LENGTH];
}

CDatabaseIO::~CDatabaseIO()
{
	SafeDeleteArray (blob_query_buffer);
}

bool CDatabaseIO::VerifyAccount(const char *username, const char *password, AccountInformation_t *pAccountInfo, int *pError)
{
	*pError = DBIO_ERROR_NONE;

	std::string usernameEscaped = g_pDB2->EscapeString(username);

	std::string passwordSalt;
	if (g_pDB2->Query("SELECT password_salt FROM accounts WHERE username='%s' LIMIT 1", usernameEscaped.c_str()))
	{
		CSQLResult *pResult = g_pDB2->GetResult();
		if (pResult)
		{
			SQLResultRow_t ResultRow = pResult->FetchRow();

			if (ResultRow)
			{
				passwordSalt = ResultRow[0];
				delete pResult;
			}
			else
			{
				delete pResult;

				*pError = VERIFYACCOUNT_ERROR_DOESNT_EXIST;
				return false;
			}
		}
		else
		{
			*pError = VERIFYACCOUNT_ERROR_QUERY_FAILED;
			return false;
		}
	}
	else
	{
		*pError = VERIFYACCOUNT_ERROR_QUERY_FAILED;
		return false;
	}

	bool bIsAdmin = false;
	if (!_stricmp(username, "admin"))
	{
		if (!strcmp(password, csprintf("%I64u", g_RandomAdminPassword)))
		{
			bIsAdmin = true;
			//*pError = VERIFYACCOUNT_ERROR_BAD_LOGIN;
			//return false;
		}

		//bIsAdmin = true;
	}

	std::string saltedPassword = std::string(password) + passwordSalt;
	std::string hashedSaltedPassword = SHA512(saltedPassword.c_str(), saltedPassword.length()).substr(0, 64);
	if (g_pDB2->Query("SELECT id, username, date_created, access FROM accounts WHERE (username!='admin' AND username='%s' AND password='%s') OR (username='admin' AND %d) LIMIT 1", usernameEscaped.c_str(), hashedSaltedPassword.c_str(), bIsAdmin))
	{
		CSQLResult *pResult = g_pDB2->GetResult();
		if (pResult)
		{
			SQLResultRow_t ResultRow = pResult->FetchRow();

			if (ResultRow)
			{
				pAccountInfo->id = CSQLResult::SafeUInt(ResultRow[0]);
				pAccountInfo->username = CSQLResult::SafeString(ResultRow[1]);
				pAccountInfo->dateCreated = CSQLResult::SafeUInt(ResultRow[2]);
				pAccountInfo->access = CSQLResult::SafeUInt(ResultRow[3]);
				delete pResult;
			}
			else
			{
				delete pResult;

				*pError = VERIFYACCOUNT_ERROR_BAD_LOGIN;
				return false;
			}
		}
		else
		{
			*pError = VERIFYACCOUNT_ERROR_QUERY_FAILED;
			return false;
		}
	}
	else
	{
		*pError = VERIFYACCOUNT_ERROR_QUERY_FAILED;
		return false;
	}

	return true;
}

bool CDatabaseIO::CreateAccount(const char *username, const char *password, int *pError, const char *ipaddress)
{
	unsigned int usernameLength = (unsigned int)strlen(username);
	if (usernameLength < MIN_USERNAME_LENGTH || usernameLength > MAX_USERNAME_LENGTH)
	{
		*pError = CREATEACCOUNT_ERROR_BAD_USERNAME;
		return false;
	}

	// Check if username is valid characters.
	for (char &c : std::string(username))
	{
		if (c >= 'a' && c <= 'z')
			continue;
		if (c >= 'A' && c <= 'Z')
			continue;
		if (c >= '0' && c <= '9')
			continue;
		if (c == '_')
			continue;
		if (c == '-')
			continue;

		*pError = CREATEACCOUNT_ERROR_BAD_USERNAME;
		return false;
	}

	unsigned int passwordLength = (unsigned int)strlen(password);
	if (passwordLength < MIN_PASSWORD_LENGTH || passwordLength > MAX_PASSWORD_LENGTH)
	{
		*pError = CREATEACCOUNT_ERROR_BAD_PASSWORD;
		return false;
	}

	*pError = DBIO_ERROR_NONE;

	std::string usernameEscaped = g_pDB2->EscapeString(username);
	std::string ipaddressEscaped = g_pDB2->EscapeString(ipaddress);

	// Create random salt
	DWORD randomValue = Random::GenUInt(0, 100000);
	std::string passwordSalt = SHA512(&randomValue, sizeof(randomValue)).substr(0, 16);

	// Apply salt
	std::string saltedPassword = password + passwordSalt;
	std::string hashedSaltedPassword = SHA512(saltedPassword.c_str(), saltedPassword.length()).substr(0, 64);

	uint64_t accountID = 0;
	if (g_pDB2->Query("INSERT INTO accounts (username, password, password_salt, date_created, access, created_ip_address) VALUES ('%s', '%s', '%s', UNIX_TIMESTAMP(), 1, '%s')", usernameEscaped.c_str(), hashedSaltedPassword.c_str(), passwordSalt.c_str(), ipaddressEscaped.c_str()))
	{
		accountID = g_pDB2->GetInsertID();

		if (!accountID)
		{
			*pError = CREATEACCOUNT_ERROR_QUERY_FAILED;
			return false;
		}
	}
	else
	{
		*pError = CREATEACCOUNT_ERROR_QUERY_FAILED;
		return false;
	}

	return true;
}

std::list<unsigned int> CDatabaseIO::GetWeeniesAt(unsigned int block_id)
{
	std::list<unsigned int> results;

	if (g_pDB2->Query("SELECT weenie_id FROM blocks WHERE block_id = %u", block_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			while (SQLResultRow_t Row = pQueryResult->FetchRow())
			{
				results.push_back(strtoul(Row[0], NULL, 10));
			}

			delete pQueryResult;
		}
	}

	return results;
}

bool CDatabaseIO::AddOrUpdateWeenieToBlock(unsigned int weenie_id, unsigned int block_id)
{
	return g_pDB2->Query("INSERT INTO blocks (weenie_id, block_id) VALUES (%u, %u) ON DUPLICATE KEY UPDATE block_id = %u", weenie_id, block_id, block_id);
}

bool CDatabaseIO::RemoveWeenieFromBlock(unsigned int weenie_id)
{
	return g_pDB2->Query("DELETE FROM blocks WHERE weenie_id = %u", weenie_id);
}

std::list<CharacterDesc_t> CDatabaseIO::GetCharacterList(unsigned int account_id)
{
	std::list<CharacterDesc_t> results;

	if (g_pDB2->Query("SELECT account_id, weenie_id, name, date_created, instance_ts FROM characters WHERE account_id = %u", account_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			while (SQLResultRow_t Row = pQueryResult->FetchRow())
			{
				CharacterDesc_t entry;
				entry.account_id = strtoul(Row[0], NULL, 10);
				entry.weenie_id = strtoul(Row[1], NULL, 10);
				entry.name = Row[2];
				entry.date_created = strtoul(Row[3], NULL, 10);
				entry.instance_ts = (WORD)strtoul(Row[4], NULL, 10);
				results.push_back(entry);
			}

			delete pQueryResult;
		}
	}

	return results;
}

bool CDatabaseIO::CreateCharacter(unsigned int account_id, unsigned int weenie_id, const char *name)
{
	return g_pDB2->Query("INSERT INTO characters (account_id, weenie_id, name, date_created, instance_ts) VALUES (%u, %u, '%s', UNIX_TIMESTAMP(), 1)",
		account_id, weenie_id, g_pDB2->EscapeString(name).c_str());
}

bool CDatabaseIO::DeleteCharacter(unsigned int weenie_id)
{
	return g_pDB2->Query("DELETE FROM characters WHERE weenie_id = %u", weenie_id);
}

bool CDatabaseIO::SetCharacterInstanceTS(unsigned int weenie_id, unsigned int instance_ts)
{
	return g_pDB2->Query("UPDATE characters SET instance_ts = %u WHERE weenie_id = %u", instance_ts, weenie_id);
}

unsigned int CDatabaseIO::GetHighestWeenieID(unsigned int min_range, unsigned int max_range)
{
	unsigned int result = min_range;

	if (g_pDB2->Query(csprintf("SELECT id FROM weenies WHERE id >= %u AND id < %u ORDER BY id DESC LIMIT 1", min_range, max_range)))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			if (SQLResultRow_t Row = pQueryResult->FetchRow())
				result = strtoul(Row[0], NULL, 10);

			delete pQueryResult;
			return result;
		}
	}

	LOG(Database, Error, "Could not get highest weenie ID for range 0x%08X - 0x%08X, returning 0x%08X.\n", min_range, max_range, min_range);
	return result;
}

bool CDatabaseIO::IsCharacterNameOpen(const char *name)
{
	bool bIsOpen = false;

	if (g_pDB2->Query("SELECT weenie_id FROM characters WHERE name='%s'", g_pDB2->EscapeString(name).c_str()))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			bIsOpen = (pQueryResult->ResultRows() == 0);
			delete pQueryResult;
		}
	}

	return bIsOpen;
}

bool CDatabaseIO::IsPlayerCharacter(unsigned int weenie_id)
{
	bool isPlayer = false;
	if (g_pDB2->Query("SELECT weenie_id FROM characters WHERE weenie_id='%u'", weenie_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			isPlayer = (pQueryResult->ResultRows() > 0);
			delete pQueryResult;
		}
	}

	return isPlayer;
}

DWORD CDatabaseIO::GetPlayerCharacterId(const char *name)
{
	DWORD result;
	if (g_pDB2->Query("SELECT weenie_id FROM characters WHERE name='%s'", g_pDB2->EscapeString(name).c_str()))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			if (SQLResultRow_t Row = pQueryResult->FetchRow())
				result = strtoul(Row[0], NULL, 10);

			delete pQueryResult;
			return result;
		}
	}
	return 0;
}

std::string CDatabaseIO::GetPlayerCharacterName(DWORD weenie_id)
{
	std::string result;
	if (g_pDB2->Query("SELECT name FROM characters WHERE weenie_id='%u'", weenie_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			if (SQLResultRow_t Row = pQueryResult->FetchRow())
				result = Row[0];

			delete pQueryResult;
			return result;
		}
	}
	return "";
}

/*
bool CDatabaseIO::CreateOrUpdateWeenie(unsigned int weenie_id, unsigned int top_level_object_id, unsigned int block_id, void *data, unsigned int data_length)
{
	// synchronous, deprecated
	return CreateOrUpdateWeenie((MYSQL *)g_pDB2->GetInternalConnection(), weenie_id, top_level_object_id, block_id, data, data_length);
}
*/

bool CDatabaseIO::CreateOrUpdateWeenie(unsigned int weenie_id, unsigned int top_level_object_id, unsigned int block_id, void *data, unsigned int data_length)
{
	IncrementPendingSave(weenie_id);
	g_pDB2->QueueAsyncQuery(new CMYSQLSaveWeenieQuery(weenie_id, top_level_object_id, block_id, data, data_length));
	return true;
}

bool CDatabaseIO::GetWeenie(unsigned int weenie_id, unsigned int *top_level_object_id, unsigned int *block_id, void **data, unsigned long *data_length)
{
	// this is so bad...
	while (GetNumPendingSaves(weenie_id))
		Sleep(0);

	MYSQL *sql = (MYSQL *)g_pDB2->GetInternalConnection();
	if (!sql)
		return false;

	MYSQL_STMT *statement = mysql_stmt_init(sql);

	const char *query_string = csprintf("SELECT top_level_object_id, block_id, data FROM weenies WHERE id = %u", weenie_id);
	mysql_stmt_prepare(statement, query_string, (unsigned long) strlen(query_string));

	MYSQL_BIND binding[3];
	memset(binding, 0, sizeof(binding));

	binding[0].buffer = top_level_object_id;
	binding[0].buffer_length = sizeof(unsigned int);
	binding[0].buffer_type = MYSQL_TYPE_LONG;
	binding[0].is_unsigned = true;
	
	binding[1].buffer = block_id;
	binding[1].buffer_length = sizeof(unsigned int);
	binding[1].buffer_type = MYSQL_TYPE_LONG;
	binding[1].is_unsigned = true;

	binding[2].buffer = blob_query_buffer;
	binding[2].buffer_length = BLOB_QUERY_BUFFER_LENGTH;
	binding[2].buffer_type = MYSQL_TYPE_BLOB;
	binding[2].length = data_length;

	mysql_stmt_bind_result(statement, binding);

	if (mysql_stmt_execute(statement) || mysql_stmt_store_result(statement))
	{
		LOG(Database, Error, "Error on GetWeenie: %s\n", mysql_error(sql));
		mysql_stmt_close(statement);
		return false;
	}

	if (mysql_stmt_fetch(statement))
	{
		LOG(Database, Warning, "GetWeenie failed because weenie 0x%08X doesn't exist!\n%s\n", weenie_id, query_string);

		mysql_stmt_free_result(statement);
		mysql_stmt_close(statement);
		return false;
	}

	*data = blob_query_buffer;

	mysql_stmt_free_result(statement);
	mysql_stmt_close(statement);
	return true;
}

bool CDatabaseIO::DeleteWeenie(unsigned int weenie_id)
{
	return g_pDB2->Query("DELETE FROM weenies WHERE id = %u OR top_level_object_id = %u", weenie_id, weenie_id);
}

bool CDatabaseIO::IsWeenieInDatabase(unsigned int weenie_id)
{
	bool isWeenieInDatabase = false;
	if (g_pDB2->Query("SELECT id FROM weenies WHERE id ='%u'", weenie_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			isWeenieInDatabase = (pQueryResult->ResultRows() > 0);
			delete pQueryResult;
		}
	}

	return isWeenieInDatabase;
}

bool CDatabaseIO::CreateOrUpdateGlobalData(DBIOGlobalDataID id, void *data, unsigned long data_length)
{
	MYSQL *sql = (MYSQL *)g_pDB2->GetInternalConnection();
	if (!sql)
		return false;

	MYSQL_STMT *statement = mysql_stmt_init(sql);

	const char *query_string = csprintf("REPLACE INTO globals (id, data) VALUES (%u, ?)", id);
	mysql_stmt_prepare(statement, query_string, (unsigned long)strlen(query_string));

	MYSQL_BIND data_param;
	memset(&data_param, 0, sizeof(data_param));
	data_param.buffer = data;
	data_param.buffer_length = data_length;
	data_param.buffer_type = MYSQL_TYPE_BLOB;
	mysql_stmt_bind_param(statement, &data_param);

	bool bErrored = false;
	if (mysql_stmt_execute(statement))
	{
		LOG(Database, Error, "Error on CreateOrUpdateGlobalData for 0x%08X: %s\n", id, mysql_error(sql));
		bErrored = true;
	}

	mysql_stmt_close(statement);
	return !bErrored;
}

bool CDatabaseIO::GetGlobalData(DBIOGlobalDataID id, void **data, unsigned long *data_length)
{
	MYSQL *sql = (MYSQL *)g_pDB2->GetInternalConnection();
	if (!sql)
		return false;

	MYSQL_STMT *statement = mysql_stmt_init(sql);

	const char *query_string = csprintf("SELECT data FROM globals WHERE id = %u", id);
	mysql_stmt_prepare(statement, query_string, (unsigned long)strlen(query_string));

	MYSQL_BIND binding[1];
	memset(binding, 0, sizeof(binding));

	binding[0].buffer = blob_query_buffer;
	binding[0].buffer_length = BLOB_QUERY_BUFFER_LENGTH;
	binding[0].buffer_type = MYSQL_TYPE_BLOB;
	binding[0].length = data_length;

	mysql_stmt_bind_result(statement, binding);

	if (mysql_stmt_execute(statement) || mysql_stmt_store_result(statement))
	{
		LOG(Database, Error, "Error on GetGlobalData: %s\n", mysql_error(sql));
		mysql_stmt_close(statement);
		return false;
	}

	if (mysql_stmt_fetch(statement))
	{
		LOG(Database, Verbose, "GetGlobalData failed because id 0x%08X doesn't exist!\n%s\n", id, query_string);

		mysql_stmt_free_result(statement);
		mysql_stmt_close(statement);
		return false;
	}

	*data = blob_query_buffer;

	mysql_stmt_free_result(statement);
	mysql_stmt_close(statement);
	return true;
}

bool CDatabaseIO::CreateOrUpdateHouseData(unsigned int house_id, void *data, unsigned int data_length)
{
	IncrementPendingSave(house_id);
	g_pDB2->QueueAsyncQuery(new CMYSQLSaveHouseQuery(house_id, data, data_length));
	return true;
}

bool CDatabaseIO::GetHouseData(unsigned int house_id, void **data, unsigned long *data_length)
{
	// this is so bad...
	while (GetNumPendingSaves(house_id))
		Sleep(0);

	MYSQL *sql = (MYSQL *)g_pDB2->GetInternalConnection();
	if (!sql)
		return false;

	MYSQL_STMT *statement = mysql_stmt_init(sql);

	const char *query_string = csprintf("SELECT data FROM houses WHERE house_id = %u", house_id);
	mysql_stmt_prepare(statement, query_string, (unsigned long)strlen(query_string));

	MYSQL_BIND binding[1];
	memset(binding, 0, sizeof(binding));

	binding[0].buffer = blob_query_buffer;
	binding[0].buffer_length = BLOB_QUERY_BUFFER_LENGTH;
	binding[0].buffer_type = MYSQL_TYPE_BLOB;
	binding[0].length = data_length;

	mysql_stmt_bind_result(statement, binding);

	if (mysql_stmt_execute(statement) || mysql_stmt_store_result(statement))
	{
		LOG(Database, Error, "Error on GetHouseData: %s\n", mysql_error(sql));
		mysql_stmt_close(statement);
		return false;
	}

	if (mysql_stmt_fetch(statement))
	{
		mysql_stmt_free_result(statement);
		mysql_stmt_close(statement);
		return false;
	}

	*data = blob_query_buffer;

	mysql_stmt_free_result(statement);
	mysql_stmt_close(statement);
	return true;
}

// CLockable _pendingSavesLock;
// std::unordered_map<DWORD, DWORD> _pendingSaves;

void CDatabaseIO::IncrementPendingSave(DWORD weenie_id)
{
	_pendingSavesLock.Lock();

	std::unordered_map<DWORD, DWORD>::iterator i = _pendingSaves.find(weenie_id);
	if (i != _pendingSaves.end())
	{
		i->second++;
	}
	else
	{
		_pendingSaves.insert(std::pair<DWORD, DWORD>(weenie_id, 1));
	}

	_pendingSavesLock.Unlock();
}

void CDatabaseIO::DecrementPendingSave(DWORD weenie_id)
{
	_pendingSavesLock.Lock();

	std::unordered_map<DWORD, DWORD>::iterator i = _pendingSaves.find(weenie_id);
	if (i != _pendingSaves.end())
	{
		if (i->second <= 1)
			_pendingSaves.erase(i);
		else
			i->second--;
	}

	_pendingSavesLock.Unlock();
}

DWORD CDatabaseIO::GetNumPendingSaves(DWORD weenie_id)
{
	_pendingSavesLock.Lock();
	std::unordered_map<DWORD, DWORD>::iterator i = _pendingSaves.find(weenie_id);
	
	DWORD numSaves = 0;
	if (i != _pendingSaves.end())
	{
		numSaves = i->second;
	}

	_pendingSavesLock.Unlock();
	return numSaves;
}

