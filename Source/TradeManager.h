#pragma once

#include "Player.h"

class TradeManager
{
public:
	static TradeManager* RegisterTrade(CPlayerWeenie *initiator, CPlayerWeenie *partner);

	void CloseTrade(CPlayerWeenie *playerFrom, uint32_t reason = 0x51);
	void OnCloseTrade(CPlayerWeenie *player, uint32_t reason = 0);

	void AddToTrade(CPlayerWeenie *playerFrom, uint32_t item);
	//void RemoveFromTrade(CPlayerWeenie *playerFrom, uint32_t item);

	void AcceptTrade(CPlayerWeenie *playerFrom);
	bool OnTradeAccepted();

	void DeclineTrade(CPlayerWeenie *playerFrom);

	void ResetTrade(CPlayerWeenie *playerFrom);

	void TradeFailure(CPlayerWeenie *playerFrom, uint32_t item);

	CPlayerWeenie* GetOtherPlayer(CPlayerWeenie *player);

	void CheckDistance();
private:
	// prevent anyone from initiating this outside of OpenTrade
	TradeManager(CPlayerWeenie *initiator, CPlayerWeenie *partner);

	void Delete();

	CPlayerWeenie *_initiator;
	CPlayerWeenie *_partner;

	bool m_bInitiatorAccepted = false;
	bool m_bPartnerAccepted = false;

	std::list<uint32_t> m_lInitiatorItems;
	std::list<uint32_t> m_lPartnerItems;

	// double check the trade is still legit
	bool CheckTrade();
};