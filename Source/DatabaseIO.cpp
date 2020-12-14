
#include <StdAfx.h>
#include "Database2.h"
#include "DatabaseIO.h"
#include "SHA512.h"
#include "Config.h"

uint64_t g_RandomAdminPassword = 0; // this is used for the admin login

CDatabaseIO::CDatabaseIO()
{
	blob_query_buffer = new BYTE[BLOB_QUERY_BUFFER_LENGTH];
}

CDatabaseIO::~CDatabaseIO()
{
	SafeDeleteArray (blob_query_buffer);
}

bool CDatabaseIO::IsUserNameValid(const std::string &username)
{
	for (const char &c : username)
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

		return false;
	}

	return true;
}

bool CDatabaseIO::VerifyAccount(const char *username, const char *password, AccountInformation_t *pAccountInfo, int *pError)
{
	return VerifyAccount(g_pDB2->GetInternalConnection(), username, password, pAccountInfo, pError);
}

bool CDatabaseIO::VerifyAccount(void* dbc, const char *username, const char *password, AccountInformation_t *pAccountInfo, int *pError)
{
	*pError = DBIO_ERROR_NONE;

	std::string salt;
	{
		mysql_statement<1> salt_query((MYSQL*)dbc, "SELECT password_salt FROM accounts WHERE username=? LIMIT 1");
		if (!salt_query)
		{
			*pError = VERIFYACCOUNT_ERROR_QUERY_FAILED;
			return false;
		}

		salt_query.bind(0, username);

		if (!salt_query.execute())
		{
			*pError = VERIFYACCOUNT_ERROR_QUERY_FAILED;
			return false;
		}

		salt.resize(64);
		unsigned long length = 0;

		mysql_statement_results<1> salt_row;
		salt_row.bind(0, salt, salt.capacity(), &length);

		if (!salt_query.bindResults(salt_row))
		{
			*pError = VERIFYACCOUNT_ERROR_QUERY_FAILED;
			return false;
		}

		if (!salt_row.next())
		{
			*pError = VERIFYACCOUNT_ERROR_DOESNT_EXIST;
			return false;
		}

		salt.resize(length);
	}

	bool bIsAdmin = false;
	if (!stricmp(username, "admin"))
	{
		if (!strcmp(password, csprintf("%I64u", g_RandomAdminPassword)))
		{
			bIsAdmin = true;
			//*pError = VERIFYACCOUNT_ERROR_BAD_LOGIN;
			//return false;
		}

		//bIsAdmin = true;
	}

	std::string pass = password + salt;
	std::string hashed = SHA512(pass.c_str(), pass.length()).substr(0, 64);

	mysql_statement<3> login_query((MYSQL*)dbc, "SELECT id, date_created, access, banned FROM accounts WHERE username=? AND password=? LIMIT 1");
	if (!login_query)
		return true;

	login_query.bind(0, username);
	login_query.bind(1, hashed);

	if (login_query.execute())
	{
		uint16_t banned = 0;

		mysql_statement_results<4> login_row;
		login_row.bind(0, pAccountInfo->id);
		login_row.bind(1, pAccountInfo->dateCreated);
		login_row.bind(2, pAccountInfo->access);
		login_row.bind(3, banned);

		if (login_query.bindResults(login_row))
		{
			if (login_row.next())
			{
				pAccountInfo->username = username;
				pAccountInfo->banned = banned != 0;
				return true;
			}
			else
			{
				*pError = VERIFYACCOUNT_ERROR_BAD_LOGIN;
				return false;
			}
		}
	}
	*pError = VERIFYACCOUNT_ERROR_QUERY_FAILED;
	return false;
}

bool CDatabaseIO::CreateAccount(const char *username, const char *password, int *pError, const char *ipaddress)
{
	return CreateAccount(g_pDB2->GetInternalConnection(), username, password, pError, ipaddress);
}

bool CDatabaseIO::CreateAccount(void* dbc, const char *username, const char *password, int *pError, const char *ipaddress)
{
	unsigned int usernameLength = (unsigned int)strlen(username);
	if (usernameLength < MIN_USERNAME_LENGTH || usernameLength > MAX_USERNAME_LENGTH)
	{
		*pError = CREATEACCOUNT_ERROR_BAD_USERNAME;
		return false;
	}

	if (!IsUserNameValid(username))
	{
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

	// Create random salt
	uint32_t randomValue = Random::GenUInt(0, 100000);
	std::string passwordSalt = SHA512(&randomValue, sizeof(randomValue)).substr(0, 16);

	// Apply salt
	std::string saltedPassword = password + passwordSalt;
	std::string hashedSaltedPassword = SHA512(saltedPassword.c_str(), saltedPassword.length()).substr(0, 64);

	uint64_t accountID = 0;
	mysql_statement<4> query((MYSQL*)dbc, "INSERT INTO accounts (username, password, password_salt, date_created, access, created_ip_address) VALUES (?, ?, ?, UNIX_TIMESTAMP(), 1, ?)");
	if (!query)
		return false;

	query.bind(0, username);
	query.bind(1, hashedSaltedPassword);
	query.bind(2, passwordSalt);
	query.bind(3, ipaddress);

	if (query.execute())
	{
		accountID = query.lastInsertId();
	}

	if (!accountID)
	{
		*pError = CREATEACCOUNT_ERROR_QUERY_FAILED;
		return false;
	}

	return true;
}

bool CDatabaseIO::UpdateBan(unsigned int account_id, bool ban)
{
	//return g_pDB2->Query("UPDATE accounts SET banned = %u WHERE id = %u", ban, account_id);
	int banned = ban ? 1 : 0;
	g_pDB2->QueueAsyncQuery("UPDATE accounts SET banned = ? WHERE id = ?", banned, account_id);
	return true;
}

std::list<HouseInfo_t> CDatabaseIO::GetHousingData()
{
	std::list<HouseInfo_t> results;
	if (g_pDB2->Query("SELECT house_id, data, LENGTH(data) FROM houses;"))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			while (SQLResultRow_t Row = pQueryResult->FetchRow())
			{
				HouseInfo_t res;
				res.house_id = strtoul(Row[0], NULL, 10);
				//res.houseBlob = Row[1];
				res.houseBlobLen = strtoul(Row[2], NULL, 10);
				res.houseBlob = new uint8_t[res.houseBlobLen];
				memcpy(res.houseBlob, Row[1], res.houseBlobLen);
				
				results.push_back(res);
			}
			delete pQueryResult;
		}
	}
	return results;
}

std::list<unsigned int> CDatabaseIO::GetAllCharacterIDs()
{
	std::list<unsigned int> results;

	if (g_pDB2->Query("SELECT weenie_id FROM characters;"))
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

std::string CDatabaseIO::GetEmailOfAccount(unsigned int character_id)
{
	std::string result;

	if (g_pDB2->Query("SELECT ac.email FROM accounts ac JOIN characters cc ON cc.account_id = ac.id WHERE cc.weenie_id = %u", character_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			while (SQLResultRow_t Row = pQueryResult->FetchRow())
			{
				result = Row[0];
			}
			delete pQueryResult;
		}
	}

	return result;
}

bool CDatabaseIO::SetEmailOfAccount(std::string email, unsigned int character_id)
{
	if (!g_pDB2->Query("UPDATE accounts AS ac JOIN characters as cc on cc.account_id = ac.id SET ac.email = '%s' WHERE cc.weenie_id = %u;", email.c_str(), character_id))
		return false;
	if (!g_pDB2->Query("UPDATE accounts AS ac JOIN characters as cc on cc.account_id = ac.id SET ac.emailsetused = 1 WHERE cc.weenie_id = %u;", character_id))
		SERVER_ERROR << "Failed to set one time email flag to true for character:" << character_id << "using email" << email;

	return true;
}

bool CDatabaseIO::IsEmailAlreadySet(unsigned int character_id)
{
	bool result = false;

	if (g_pDB2->Query("SELECT count(*) FROM accounts ac JOIN characters cc ON cc.account_id = ac.id WHERE  ac.emailsetused = 1 AND cc.weenie_id = %u;", character_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			while (SQLResultRow_t Row = pQueryResult->FetchRow())
			{
				if (strtoul(Row[0], NULL, 10) > 0)
					result = true;
			}
			delete pQueryResult;
		}
	}

	return result;
}


std::list<unsigned int> CDatabaseIO::GetBannedAccountIDs()
{
	std::list<unsigned int> results;

	if (g_pDB2->Query("SELECT id FROM accounts WHERE banned = 1;"))
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

std::list<unsigned int> CDatabaseIO::GetBannedCharacterIDs()
{
	std::list<unsigned int> results;

	if (g_pDB2->Query("SELECT weenie_id FROM characters;"))
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

std::list<unsigned int> CDatabaseIO::GetWeeniesAt(unsigned int block_id)
{
	std::list<unsigned int> results;

	if (g_pDB2->Query("SELECT weenie_id FROM blocks WHERE block_id = %u;", block_id))
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

std::string CDatabaseIO::LoadCharacterTitles(unsigned int character_id)
{
	std::string result;
	if (g_pDB2->Query("SELECT titles FROM character_titles WHERE character_id = %u;", character_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			while (SQLResultRow_t Row = pQueryResult->FetchRow())
			{
				result = Row[0];
			};

			delete pQueryResult;
		}
	}

	return result;
}

std::string CDatabaseIO::LoadCharacterCorpses(unsigned int character_id)
{
	std::string result;
	if (g_pDB2->Query("SELECT corpses FROM character_corpses WHERE character_id = %u;", character_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			while (SQLResultRow_t Row = pQueryResult->FetchRow())
			{
				result = Row[0];
			};

			delete pQueryResult;
		}
	}

	return result;
}


bool CDatabaseIO::SaveCharacterTitles(unsigned int character_id, std::string titles)
{
	if (!g_pDB2->Query("REPLACE INTO character_titles (character_id, titles) VALUES (%u, '%s');", character_id, titles.c_str()))
		return false;
	return true;
}

bool CDatabaseIO::SaveCharacterCorpses(unsigned int character_id, std::string corpselist)
{
	if (!g_pDB2->Query("REPLACE INTO character_corpses (character_id, corpses) VALUES (%u, '%s');", character_id, corpselist.c_str()))
		return false;
	return true;
}



bool CDatabaseIO::DeleteCharacterTitles(unsigned int character_id)
{
	if (!g_pDB2->Query("DELETE FROM character_titles WHERE character_id = %u", character_id))
		return false;
	return true;
}

bool CDatabaseIO::DeleteCharacterInventory(unsigned int character_id)
{
	if (!g_pDB2->Query("DELETE FROM weenies WHERE top_level_object_id = %u", character_id))
	{
		SERVER_ERROR << "CHAR DELETE: Failed to delete inventory for weenie - " << character_id;
		return false;
	}
	return true;
}

bool CDatabaseIO::AddOrUpdateWeenieToBlock(unsigned int weenie_id, unsigned int block_id)
{
	return g_pDB2->Query("INSERT INTO blocks (weenie_id, block_id) VALUES (%u, %u) ON DUPLICATE KEY UPDATE block_id = %u;", weenie_id, block_id, block_id);
}

bool CDatabaseIO::RemoveWeenieFromBlock(unsigned int weenie_id)
{
	return g_pDB2->Query("DELETE FROM blocks WHERE weenie_id = %u;", weenie_id);
}

std::list<CharacterDesc_t> CDatabaseIO::GetCharacterList(unsigned int account_id)
{
	std::list<CharacterDesc_t> results;
	std::vector<unsigned int> charsToDelete;
	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT account_id, weenie_id, name, date_created, instance_ts, ts_deleted, gag_timer FROM characters WHERE account_id = ? ORDER BY ts_login DESC;",
		account_id);

	if (statement)
	{
		CharacterDesc_t desc = { 0, 0, "", 0, 0, 0, 0.0};
		
		desc.name.resize(64);
		unsigned long length = 0;

		mysql_statement_results<7> row;
		row.bind(0, desc.account_id);
		row.bind(1, desc.weenie_id);
		row.bind(2, desc.name, desc.name.capacity(), &length);
		row.bind(3, desc.date_created);
		row.bind(4, desc.instance_ts);
		row.bind(5, desc.ts_deleted);
		row.bind(6, desc.gag_timer);


		if (statement.bindResults(row))
		{
			while (row.next())
			{
				int32_t ts_delete = 0;
				if (desc.ts_deleted)
				{
					auto now = std::chrono::system_clock::now();
					uint32_t ts = (uint32_t)std::chrono::system_clock::to_time_t(now);
					ts_delete = g_pConfig->GetDeleteCharLifespan() - (ts - desc.ts_deleted);
					if (ts_delete <= 0)
					{
						charsToDelete.push_back(desc.weenie_id);
						continue;
					}
					desc.ts_deleted = ts_delete;
				}
				CharacterDesc_t entry = desc;
				entry.name.resize(length);
				results.push_back(entry);
			}
		}
	}
	for (auto it = charsToDelete.begin(); it != charsToDelete.end(); ++it) {
		DeleteCharacter(*it, true);
	}
	
	return results;
}



std::list<CharacterSquelch_t> CDatabaseIO::GetCharacterSquelch(unsigned int character_id)
{
	std::list<CharacterSquelch_t> results;

	if (g_pDB2->Query("SELECT squelched_id, account_id, isip, isspeech, istell, iscombat, ismagic, isemote, isadvancement, isappraisal, isspellcasting, isallegiance, isfellowhip, iscombatenemy, isrecall, iscrafting FROM character_squelch WHERE character_id = %u", character_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			while (SQLResultRow_t Row = pQueryResult->FetchRow())
			{
				CharacterSquelch_t entry;
				entry.squelched_id = strtoul(Row[0], NULL, 10);
				entry.account_id = strtoul(Row[1], NULL, 10);
				entry.isIp = Row[2];
				entry.isSpeech = Row[3];
				entry.isTell = Row[4];
				entry.isCombat = Row[5];
				entry.isMagic = Row[6];
				entry.isEmote = Row[7];
				entry.isAdvancement = Row[8];
				entry.isAppraisal = Row[9];
				entry.isSpellcasting = Row[10];
				entry.isAllegiance = Row[11];
				entry.isFellowship = Row[12];
				entry.isCombatEnemy = Row[13];
				entry.isRecall = Row[14];
				entry.isCrafting = Row[15];
				results.push_back(entry);
			}
			delete pQueryResult;
		}
	}

	return results;
}

bool CDatabaseIO::SaveCharacterSquelch(unsigned int character_id, CharacterSquelch_t data)
{
	return g_pDB2->Query("INSERT INTO character_squelch(character_id, squelched_id, account_id, isip, isspeech, istell, iscombat, ismagic, isemote, isadvancement, isappraisal, isspellcasting, isallegiance, isfellowhip, iscombatenemy, isrecall, iscrafting) VALUES"
		"(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u) ON DUPLICATE KEY UPDATE "
		"isip = %u, isspeech = %u, istell = %u, iscombat = %u, ismagic = %u, isemote = %u, isadvancement = %u, isappraisal = %u, isspellcasting = %u, isallegiance = %u, isfellowhip = %u, iscombatenemy = %u, isrecall = %u, iscrafting = %u;",
		character_id, data.squelched_id, data.account_id, data.isIp, data.isSpeech, data.isTell, data.isCombat, data.isMagic, data.isEmote, data.isAdvancement,
		data.isAppraisal, data.isSpellcasting, data.isAllegiance, data.isFellowship, data.isCombatEnemy, data.isRecall, data.isCrafting, 
		data.isIp, data.isSpeech, data.isTell, data.isCombat, data.isMagic, data.isEmote, data.isAdvancement,
		data.isAppraisal, data.isSpellcasting, data.isAllegiance, data.isFellowship, data.isCombatEnemy, data.isRecall, data.isCrafting);
}

bool CDatabaseIO::RemoveCharacterSquelch(unsigned int character_id, CharacterSquelch_t data)
{
	return g_pDB2->Query("DELETE FROM character_squelch WHERE character_id = %u AND squelched_id = %u;", character_id, data.squelched_id);
}

std::list<CharacterGags_t> CDatabaseIO::GetServerGagData()
{
	std::list<CharacterGags_t> results;
	mysql_statement<0> statement = g_pDB2->QueryEx(
		"SELECT name, gag_timer, gag_count FROM characters WHERE gag_timer != 0 ORDER BY name;");

	if (statement)
	{
		CharacterGags_t desc = { "", 0, 0 };

		desc.name.resize(64);
		unsigned long length = 0;

		mysql_statement_results<3> row;
		row.bind(0, desc.name, desc.name.capacity(), &length);
		row.bind(1, desc.gag_timer);
		row.bind(2, desc.gag_count);

		if (statement.bindResults(row))
		{
			while (row.next())
			{
				CharacterGags_t entry = desc;
				entry.name.resize(length);
				results.push_back(entry);
			}
		}
	}

	return results;
}

CharacterGags_t CDatabaseIO::GetCharacterGagData(unsigned int character_id)
{
	CharacterGags_t result = { "",0,0};

	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT name, gag_timer, gag_count FROM characters WHERE weenie_id = ?;", character_id);

	if (statement)
	{
		result.name.resize(64);
		unsigned long length = 0;

		mysql_statement_results<3> row;
		row.bind(0, result.name, result.name.capacity(), &length);
		row.bind(1, result.gag_timer);
		row.bind(2, result.gag_count);

		if (statement.bindResults(row))
		{
			if (row.next())
			{
				result.name.resize(length);
			}
		}
	}
	return result;
}

bool CDatabaseIO::SaveServerGag(unsigned int character_id, double duration, bool account)
{
	int gagCount = 1;
	if (g_pDB2->Query("SELECT gag_count FROM characters WHERE weenie_id = %u;", character_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			SQLResultRow_t Row = pQueryResult->FetchRow();
			if (Row)
			{
				gagCount = strtoul(Row[0], NULL, 10);
			}
			delete pQueryResult;
		}
	}

	if (!g_pDB2->Query("UPDATE characters SET gag_timer = %f, gag_count = %d WHERE weenie_id=%u", duration, ++gagCount, character_id))
	{
		SERVER_ERROR << "GAG UPDATE: Failed on Weenie ID -" << character_id;
		return false;
	}

	if (account)
	{
		uint32_t accountId = GetPlayerAccountId(character_id);

		std::vector<unsigned int> characters;
		if (g_pDB2->Query("SELECT weenie_id FROM characters where account_id = %u;", accountId))
		{
			CSQLResult *pQueryResult = g_pDB2->GetResult();
			if (pQueryResult)
			{
				while (SQLResultRow_t Row = pQueryResult->FetchRow())
				{
					characters.push_back(strtoul(Row[0], NULL, 10));
				}

				delete pQueryResult;
			}
		}

		for (auto chr : characters)
			SaveServerGag(chr, duration);
	}


	return true;
}

bool CDatabaseIO::RemoveServerGag(unsigned int character_id, bool account)
{
	if (!g_pDB2->Query("UPDATE characters SET gag_timer = 0.0 WHERE weenie_id=%u", character_id))
	{
		SERVER_ERROR << "GAG REMOVE: Failed on Weenie ID -" << character_id;
		return false;
	}

	if (account)
	{
		uint32_t accountId = GetPlayerAccountId(character_id);

		std::vector<unsigned int> characters;
		if (g_pDB2->Query("SELECT weenie_id FROM characters where account_id = %u;", accountId))
		{
			CSQLResult *pQueryResult = g_pDB2->GetResult();
			if (pQueryResult)
			{
				while (SQLResultRow_t Row = pQueryResult->FetchRow())
				{
					characters.push_back(strtoul(Row[0], NULL, 10));
				}

				delete pQueryResult;
			}
		}

		for (auto chr : characters)
			RemoveServerGag(chr);
	}




	return true;
}

uint32_t CDatabaseIO::GetPlayerAccountId(unsigned int character_id)
{
	uint32_t accountToSquelch = 0;

	if (g_pDB2->Query("SELECT account_id FROM characters WHERE weenie_id = %u;", character_id))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			SQLResultRow_t Row = pQueryResult->FetchRow();
			if (Row)
			{
				accountToSquelch = strtoul(Row[0], NULL, 10);
			}
			delete pQueryResult;
		}
	}

	return accountToSquelch;
}


CharacterDesc_t CDatabaseIO::GetCharacterInfo(unsigned int weenie_id)
{
	CharacterDesc_t result = { 0,0,"",0,0, 0};

	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT account_id, weenie_id, name, date_created, instance_ts, ts_deleted FROM characters WHERE weenie_id = ?;",
		weenie_id);

	if (statement)
	{
		result.name.resize(64);
		unsigned long length = 0;
		
		mysql_statement_results<6> row;
		row.bind(0, result.account_id);
		row.bind(1, result.weenie_id);
		row.bind(2, result.name, result.name.capacity(), &length);
		row.bind(3, result.date_created);
		row.bind(4, result.instance_ts);
		row.bind(5, result.ts_deleted);

		if (statement.bindResults(row))
		{
			if (row.next())
			{
				result.name.resize(length);
			}
		}
	}
	return result;
}

bool CDatabaseIO::CreateCharacter(unsigned int account_id, unsigned int weenie_id, const char *name)
{
	return g_pDB2->Query("INSERT INTO characters (account_id, weenie_id, name, date_created, instance_ts) VALUES (%u, %u, '%s', UNIX_TIMESTAMP(), 1)",
		account_id, weenie_id, g_pDB2->EscapeString(name).c_str());
}

bool CDatabaseIO::DeleteCharacter(unsigned int weenie_id, bool force)
{
	if (force)
	{
		DeleteCharacterTitles(weenie_id);
		DeleteCharacterInventory(weenie_id);
		return g_pDB2->Query("DELETE FROM characters WHERE weenie_id = %u", weenie_id);
	}

	auto now = std::chrono::system_clock::now();
	uint32_t ts = (uint32_t)std::chrono::system_clock::to_time_t(now);
	if (!g_pDB2->Query("UPDATE characters SET ts_deleted=%u WHERE weenie_id=%u", ts, weenie_id))
	{
		SERVER_ERROR << "CHAR DELETE: Failed on Weenie ID -" << weenie_id;
		return false;
	}
	return true;
}

bool CDatabaseIO::IsCharacterDeleting(unsigned int weenie_id)
{
	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT ts_deleted FROM characters WHERE ts_deleted > 0 AND weenie_id = ?",
		weenie_id);

	if (statement)
	{
		uint32_t result_id = 0;
		mysql_statement_results<1> result;
		result.bind(0, result_id);

		if (statement.bindResults(result))
		{
			if (result.next())
			{
				return true;
			}
		}
	}

	return false;
}

bool CDatabaseIO::RestoreCharacter(unsigned int weenie_id)
{
	if (!g_pDB2->Query("UPDATE characters SET ts_deleted = 0 WHERE weenie_id = %u;", weenie_id))
		return false;
	return true;
}

bool CDatabaseIO::SetCharacterInstanceTS(unsigned int weenie_id, unsigned int instance_ts)
{
	auto now = std::chrono::system_clock::now();
	uint32_t ts = (uint32_t)std::chrono::system_clock::to_time_t(now);
	auto statement = g_pDB2->Query("UPDATE characters SET instance_ts=%u, ts_login=%u WHERE weenie_id=%u;", instance_ts, ts, weenie_id);
	if (statement)
		return true;

	SERVER_ERROR << "CHAR LOGIN: Failed to set Instance or Login timestampes for Weenie ID -" << weenie_id;
	return false;
}

bool CDatabaseIO::IDRangeTableExistsAndValid()
{
	bool retval = false;

	mysql_statement<0> statement = g_pDB2->QueryEx("SELECT unused FROM idranges WHERE unused > 2147999999 LIMIT 1");
	if (statement)
	{
		unsigned int current = 0;
		mysql_statement_results<1> result;
		result.bind(0, current);

		if (statement.bindResults(result))
		{
			if (result.next())
			{
				retval = true;
			}
		}
	}

	return retval;
}

std::list<unsigned int> CDatabaseIO::GetNextIDRange(unsigned int rangeStart, unsigned int count)
{
	mysql_statement<2> statement = g_pDB2->QueryEx(
		"SELECT unused FROM idranges WHERE unused > ? limit ?;",
		rangeStart, count);

	std::list< unsigned int> found;
	if (statement)
	{
		unsigned int current = 0;
		mysql_statement_results<1> result;
		result.bind(0, current);

		if (statement.bindResults(result))
		{
			while (result.next())
			{
				found.push_back(current);
			}
		}
	}

	return found;
}

unsigned int CDatabaseIO::GetHighestWeenieID(unsigned int min_range, unsigned int max_range)
{
	uint32_t minId = min_range;

	if (g_pDB2->Query("SELECT COALESCE(max(id), %u) FROM weenies WHERE id >= %u AND id < %u;", min_range, min_range, max_range))
	{
		CSQLResult *pQueryResult = g_pDB2->GetResult();
		if (pQueryResult)
		{
			SQLResultRow_t Row = pQueryResult->FetchRow();
			if (Row)
			{
				minId = strtoul(Row[0], NULL, 10);
			}
			delete pQueryResult;
		}
	}

	return minId;
}

bool CDatabaseIO::IsCharacterNameOpen(const char *name)
{
	std::string tmp(name);
	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT weenie_id FROM characters WHERE name = ?",
		tmp);

	if (statement)
	{
		unsigned int id = 0;
		mysql_statement_results<1> result;
		result.bind(0, id);

		if (statement.bindResults(result))
		{
			if (result.next())
			{
				return false;
			}
			return true;
		}
	}

	SERVER_ERROR << "Error on IsCharacterNameOpen:" << statement.error();
	return false;
}

bool CDatabaseIO::IsPlayerCharacter(unsigned int weenie_id)
{
	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT weenie_id FROM characters WHERE weenie_id = ?",
		weenie_id);

	if (statement)
	{
		uint32_t result_id = 0;
		mysql_statement_results<1> result;
		result.bind(0, result_id);

		if (statement.bindResults(result))
		{
			if (result.next())
			{
				return true;
			}
		}
	}

	return false;
}

uint32_t CDatabaseIO::GetPlayerCharacterId(const char *name)
{
	uint32_t id = 0;

	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT weenie_id FROM characters WHERE name = ?",
		name);

	if (statement)
	{
		mysql_statement_results<1> result;
		result.bind(0, id);

		if (statement.bindResults(result))
		{
			result.next();
		}
	}

	return (uint32_t)id;
}

std::string CDatabaseIO::GetPlayerCharacterName(uint32_t weenie_id)
{
	std::string result;

	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT name FROM characters WHERE weenie_id = ?",
		weenie_id);

	if (statement)
	{
		unsigned long length = 0;
		mysql_statement_results<1> results;
		results.bind(0, result, 64, &length);

		if (statement.bindResults(results))
		{
			if (results.next())
			{
				result.resize(length);
			}
		}
	}

	return result;
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
		std::this_thread::yield();

	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT top_level_object_id, block_id, data FROM weenies WHERE id = ?",
		weenie_id);

	if (statement)
	{
		mysql_statement_results<3> result;
		result.bind(0, top_level_object_id);
		result.bind(1, block_id);
		result.bind(2, blob_query_buffer, BLOB_QUERY_BUFFER_LENGTH, data_length);

		if (statement.bindResults(result))
		{
			if (result.next())
			{
				*data = blob_query_buffer;
				return true;
			}
		}
	}

	SERVER_ERROR << "Error on GetWeenie:" << statement.error();
	return false;
}

bool CDatabaseIO::DeleteWeenie(unsigned int weenie_id)
{
	mysql_statement<2> statement = g_pDB2->QueryEx(
		"DELETE FROM weenies WHERE id = ?;",
		weenie_id, weenie_id);

	if (statement)
	{
		return true;
	}

	SERVER_ERROR << "Error on DeleteWeenie: " << statement.error();
	return false;

}

bool CDatabaseIO::IsWeenieInDatabase(unsigned int weenie_id)
{
	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT id FROM weenies WHERE id = ?",
		weenie_id);

	if (statement)
	{
		uint32_t result_id = 0;
		mysql_statement_results<1> result;
		result.bind(0, result_id);

		if (statement.bindResults(result))
		{
			if (result.next())
			{
				return true;
			}
		}
	}

	return false;
}

bool CDatabaseIO::CreateOrUpdateGlobalData(DBIOGlobalDataID id, void *data, unsigned long data_length)
{
	mysql_statement<2> statement = g_pDB2->CreateQuery<2>("REPLACE INTO globals (id, data) VALUES (?, ?)");
	
	if (statement)
	{
		uint32_t gid = (uint32_t)id;
		statement.bind(0, gid);
		statement.bind(1, data, data_length);

		if (statement.execute())
			return true;
	}
	
	SERVER_ERROR << "Error on CreateOrUpdateGlobalData for" << id << ":" << statement.error();
	return false;
}

bool CDatabaseIO::GetGlobalData(DBIOGlobalDataID id, void **data, unsigned long *data_length)
{
	uint32_t gid = (uint32_t)id;
	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT data FROM globals WHERE id = ?",
		gid);

	if (statement)
	{
		mysql_statement_results<1> result;
		result.bind(0, blob_query_buffer, BLOB_QUERY_BUFFER_LENGTH, data_length);

		if (statement.bindResults(result))
		{
			if (result.next())
			{
				*data = blob_query_buffer;
				return true;
			}

			return false;
		}
	}

	SERVER_ERROR << "Error on GetGlobalData:" << statement.error();
	return false;
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
		std::this_thread::yield();

	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT data FROM houses WHERE house_id = ?",
		house_id);

	if (statement)
	{
		mysql_statement_results<1> result;
		result.bind(0, blob_query_buffer, BLOB_QUERY_BUFFER_LENGTH, data_length);

		if (statement.bindResults(result))
		{
			if (result.next())
			{
				*data = blob_query_buffer;
				return true;
			}

			return false;
		}
	}

	SERVER_ERROR << "Error on GetHouseData:" << statement.error();
	return false;
}

// CLockable _pendingSavesLock;
// std::unordered_map<uint32_t, uint32_t> _pendingSaves;

void CDatabaseIO::IncrementPendingSave(uint32_t weenie_id)
{
	_pendingSavesLock.Lock();

	std::unordered_map<uint32_t, uint32_t>::iterator i = _pendingSaves.find(weenie_id);
	if (i != _pendingSaves.end())
	{
		i->second++;
	}
	else
	{
		_pendingSaves.insert(std::pair<uint32_t, uint32_t>(weenie_id, 1));
	}

	_pendingSavesLock.Unlock();
}

void CDatabaseIO::DecrementPendingSave(uint32_t weenie_id)
{
	_pendingSavesLock.Lock();

	std::unordered_map<uint32_t, uint32_t>::iterator i = _pendingSaves.find(weenie_id);
	if (i != _pendingSaves.end())
	{
		if (i->second <= 1)
			_pendingSaves.erase(i);
		else
			i->second--;
	}

	_pendingSavesLock.Unlock();
}

uint32_t CDatabaseIO::GetNumPendingSaves(uint32_t weenie_id)
{
	_pendingSavesLock.Lock();
	std::unordered_map<uint32_t, uint32_t>::iterator i = _pendingSaves.find(weenie_id);
	
	uint32_t numSaves = 0;
	if (i != _pendingSaves.end())
	{
		numSaves = i->second;
	}

	_pendingSavesLock.Unlock();
	return numSaves;
}

bool CDatabaseIO::IsAlreadyFriend(unsigned int character_id, unsigned int friend_id)
{
	mysql_statement<2> statement = g_pDB2->QueryEx(
		"SELECT friend_id FROM character_friends WHERE character_id = ? AND friend_id = ? AND friend_type = 1;", character_id, friend_id);

	if (statement)
	{
		unsigned int id = 0;
		mysql_statement_results<1> result;
		result.bind(0, id);

		if (statement.bindResults(result))
		{
			if (result.next())
			{
				return true;
			}
			return false;
		}
	}
	return false;
}

bool CDatabaseIO::AddCharacterFriend(unsigned int character_id, unsigned int friend_id)
{
	if (!g_pDB2->Query("REPLACE INTO character_friends (character_id, friend_type, friend_id) VALUES (%u, 1, %u);", character_id, friend_id))
		return false;

	if (!g_pDB2->Query("REPLACE INTO character_friends (character_id, friend_type, friend_id) VALUES (%u, 2, %u);", friend_id, character_id))
		return false;
	
	return true;
}

bool CDatabaseIO::RemoveCharacterFriend(unsigned int character_id, unsigned int frenemy_id)
{
	if (!g_pDB2->Query("DELETE FROM character_friends WHERE character_id = %u AND friend_id = %u;", character_id, frenemy_id))
		return false;

	if (!g_pDB2->Query("DELETE FROM character_friends WHERE friend_id = %u AND character_id = %u;", frenemy_id, character_id))
		return false;

	return true;
}

void CDatabaseIO::DeleteCharFriends(unsigned int character_id)
{
	g_pDB2->Query("DELETE FROM character_friends WHERE character_id = %u;", character_id);
	g_pDB2->Query("DELETE FROM character_friends WHERE friend_id = %u;", character_id);
}

bool CDatabaseIO::SaveWindowData(unsigned int character_id, BYTE* data, unsigned long data_length)
{
	mysql_statement<3> statement = g_pDB2->CreateQuery<3>("REPLACE INTO character_windowdata (character_id, bloblength, windowblob) VALUES (?,?, ?)");

	if (statement)
	{
		uint32_t gid = (uint32_t)character_id;
		statement.bind(0, gid);
		statement.bind(1, data_length);
		statement.bind(2, data, data_length);

		if (statement.execute())
			return true;
	}

	SERVER_ERROR << "Error on saving window data for" << character_id << ":" << statement.error();
	return false;
}

int CDatabaseIO::LoadWindowData(unsigned int character_id, BYTE ** data)
{
	int retVal = 0;
	unsigned long * data_length = 0;
	uint32_t gid = (uint32_t)character_id;
	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT bloblength, windowblob FROM character_windowdata WHERE character_id = ?",
		gid);

	if (statement)
	{
		mysql_statement_results<2> result;
		result.bind(0, retVal);
		result.bind(1, blob_query_buffer, BLOB_QUERY_BUFFER_LENGTH, data_length);

		if (statement.bindResults(result))
		{
			if (result.next())
			{
				*data = blob_query_buffer;
				return retVal;
			}

			return false;
		}
	}

	SERVER_ERROR << "Error on reading window data for" << character_id << ":" << statement.error();
	return retVal;
}

void CDatabaseIO::ClearFriendsList(unsigned int character_id)
{
	g_pDB2->Query("DELETE FROM character_friends WHERE character_id = %u;", character_id);
}

std::list<CharacterDesc_t> CDatabaseIO::GetFriends(unsigned int character_id)
{
	std::list<CharacterDesc_t > results;
	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT 0, cf.friend_id, c.name, 0, 0 FROM characters c JOIN character_friends cf ON cf.friend_id = c.weenie_id WHERE cf.friend_type = 1 AND cf.character_id = ?",
		character_id);

	if (statement)
	{
		CharacterDesc_t desc = { 0, 0, "", 0, 0 };

		desc.name.resize(64);
		unsigned long length = 0;

		mysql_statement_results<5> row;
		row.bind(0, desc.account_id);
		row.bind(1, desc.weenie_id);
		row.bind(2, desc.name, desc.name.capacity(), &length);
		row.bind(3, desc.date_created);
		row.bind(4, desc.instance_ts);

		if (statement.bindResults(row))
		{
			while (row.next())
			{
				CharacterDesc_t entry = desc;
				entry.name.resize(length);
				results.push_back(entry);
			}
		}
	}

	return results;


}

std::list<CharacterDesc_t> CDatabaseIO::GetFriendsOf(unsigned int character_id)
{
	std::list<CharacterDesc_t > results;
	mysql_statement<1> statement = g_pDB2->QueryEx(
		"SELECT 0, cf.friend_id, c.name, 0, 0 FROM characters c JOIN character_friends cf ON cf.friend_id = c.weenie_id WHERE cf.friend_type = 2 AND cf.character_id = ?",
		character_id);

	if (statement)
	{
		CharacterDesc_t desc = { 0, 0, "", 0, 0 };

		desc.name.resize(64);
		unsigned long length = 0;

		mysql_statement_results<5> row;
		row.bind(0, desc.account_id);
		row.bind(1, desc.weenie_id);
		row.bind(2, desc.name, desc.name.capacity(), &length);
		row.bind(3, desc.date_created);
		row.bind(4, desc.instance_ts);

		if (statement.bindResults(row))
		{
			while (row.next())
			{
				CharacterDesc_t entry = desc;
				entry.name.resize(length);
				results.push_back(entry);
			}
		}
	}

	return results;


}
