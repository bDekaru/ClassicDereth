
#include <StdAfx.h>
#include "Vendor.h"
#include "WeenieFactory.h"
#include "World.h"
#include "Player.h"
#include "EmoteManager.h"
#include "SpellcastingManager.h"

#define VENDOR_PURGE_TIME	(30.0 * 60.0) //Purge vendor inventory every half-hour

static Command GetRandomThanksMotion() {
	static const Command cmd[3] ={
		Motion_Wave,
		Motion_Shrug,
		Motion_BowDeep
	};
	const int max = sizeof(cmd) / sizeof(cmd[0]);

	return cmd[Random::GenUInt(0, max-1)];
}

CVendorItem::CVendorItem()
{
}

CVendorItem::~CVendorItem()
{
	g_pWorld->EnsureRemoved(weenie);
	SafeDelete(weenie);
}

CVendor::CVendor()
{
	m_Qualities.m_WeenieType = Vendor_WeenieType;
	m_NextPurgeTime = Timer::cur_time + VENDOR_PURGE_TIME;
}

CVendor::~CVendor()
{
	ResetItems();
}

void CVendor::Tick()
{
	const double timeNow = Timer::cur_time;
	if (m_EmoteManager)
		m_EmoteManager->Tick();

	if (m_VendorCycleTime <= timeNow)
	{
		CheckRange();
		m_VendorCycleTime = (timeNow + 3);
	}

	if(m_NextPurgeTime <= timeNow) {
		PurgePlayerSoldItems();
		m_NextPurgeTime = timeNow + VENDOR_PURGE_TIME;
	}

	switch(thankState) {
		case 0: //idle, find something else to do.
		{
			//Try to find a buyer whom we haven't thanked yet.
			while(!queuedThanks.empty()) {
				uint32_t buyer = queuedThanks.front();

				queuedThanks.pop_front(); //Remove this

				//If the buyer is still here, then turn to them and thank them.
				if(m_ActiveBuyers.find(buyer) != m_ActiveBuyers.end()) {
					thankState = 1;
					currentlyThanking = buyer;
					MovementParameters mp;
					TurnToObject(buyer, &mp);
					break;
				}
			}
			break;
		}

		case 1: //turning to buyer
			if(IsCompletelyIdle()) {
				DoForcedMotion(GetRandomThanksMotion());
				thankState = 2;
			}
			break;
		case 2: //performing some thanks motion
		{
			if(IsCompletelyIdle()) {
				thankState = 0; //idle
			}
		}

	}


}

void CVendor::ResetItems()
{
	for (auto item : m_Items) {
		delete item;
	}
	m_Items.clear();
}

void CVendor::AddVendorItem(uint32_t wcid, int ptid, float shade, int amount)
{
	
	if (wcid == 1)
		return;

	CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(wcid);

	if (!weenie)
		return;
	   
	if (ptid)
		weenie->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, ptid);

	if (shade > 0.0)
		weenie->m_Qualities.SetFloat(SHADE_FLOAT, shade);

	if (!g_pWorld->CreateEntity(weenie, false))
		return;

	CVendorItem *item = new CVendorItem();
	item->weenie = weenie;
	item->amount = amount;
	m_Items.push_back(item);
}

void CVendor::AddVendorItem(uint32_t wcid, int amount)
{
	if (wcid == 1)
		return;

	CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(wcid);
		
	if (!weenie)
		return;
	
	if (!g_pWorld->CreateEntity(weenie, false))
		return;

	CVendorItem *item = new CVendorItem();
	item->weenie = weenie;
	item->amount = amount;
	m_Items.push_back(item);
}

CVendorItem *CVendor::FindVendorItem(uint32_t item_id)
{
	for (auto item : m_Items) {
		if (item->weenie->GetID() == item_id)
			return item;
	}
	

	return NULL;
}

void CVendor::ResellItemToPlayer(CPlayerWeenie* player, uint32_t item_id, uint32_t amount) {
	auto iter = m_Items.begin();
	CVendorItem* itemToBuy = NULL;

	while(iter != m_Items.end() && amount > 0) {
		CVendorItem* item = *iter;
		if(item->isPlayerSold && item->weenie->GetID() == item_id) {
			itemToBuy = item;
			break;
		} else { //Not matching item, try next;
			++iter;
		}
	}

	assert(itemToBuy); //Must be found.
	//Two cases:
	//1) Player is buying all of what the vendor has. We can just give the item to the player.
	//2) Player is buying _some_ of what the vendor has. We need to clone it and give those cloned items to the player.

	if(itemToBuy->amount <= amount) {
		iter = m_Items.erase(iter);

		//Buyer now owns this.
		itemToBuy->weenie->SetWeenieContainer(player->GetID());

		//Insert into buyer's inventory
		player->Container_InsertInventoryItem(0, itemToBuy->weenie, 0);
	} else {
		itemToBuy->amount -= amount;
		player->SpawnCloneInContainer(itemToBuy->weenie, amount);
	}
}

void CVendor::AddPlayerSoldItem(CWeenieObject* weenie, uint32_t amount) {
	//Don't resell trade notes
	if(weenie->InqIntQuality(ITEM_TYPE_INT, TYPE_UNDEF) == TYPE_PROMISSORY_NOTE) {
		weenie->Remove();
		return;
	}

	//Remove items that are destroy on sell. TODO: Fix content so we can remove attuned/bonded as a proxy for destroy on sell.
	if(weenie->InqBoolQuality(DESTROY_ON_SELL_BOOL, FALSE) || weenie->InqIntQuality(BONDED_INT, 0) || weenie->InqIntQuality(ATTUNED_INT, 0)) {
		weenie->Remove();
		return;
	}

	//Check if we can merge with existing inventory. If merging was successful, we can delete this.
	if(MergeWithExistingInventory(weenie, amount)) {
		weenie->Remove();
		return;
	}

	weenie->SetWeenieContainer(this->GetID()); //vendor now owns this

	CVendorItem* item = new CVendorItem();
	item->amount = amount;
	item->weenie = weenie;
	item->isPlayerSold = true;
	m_Items.push_back(item);
}



int CVendor::TrySellItemsToPlayer(CPlayerWeenie *buyer, const std::list<ItemProfile *> &desiredItems)
{
	const uint32_t MAX_COIN_PURCHASE = 2000000000; // // limit to purchases less than 2 billion pyreal

	// using WERROR_NO_OBJECT as a generic error, change later if it matters
	if (desiredItems.size() >= 100)
		return WERROR_NO_OBJECT;

	int currencyid = W_COINSTACK_CLASS;
	if (profile.trade_id)
		currencyid = profile.trade_id;
	int currencyamount = buyer->RecalculateCoinAmount(currencyid);

	// check cost of items: Pyreal, Slots, PackSlots
	UINT64 totalCost = 0;
	uint32_t totalSlotsRequired = 0;
	uint32_t totalPackSlotsRequired = 0;
	for (auto desiredItem : desiredItems)
	{
		CVendorItem *vendorItem = FindVendorItem(desiredItem->iid);
		if (!vendorItem)
			return WERROR_NO_OBJECT;
		if (vendorItem->amount >= 0 && vendorItem->amount < desiredItem->amount)
			return WERROR_NO_OBJECT;
		if (desiredItem->amount == 0)
			continue;

		int maxStackSize = vendorItem->weenie->InqIntQuality(MAX_STACK_SIZE_INT, 1);
		if (maxStackSize < 1)
			maxStackSize = 1;

		if (vendorItem->weenie->RequiresPackSlot())
		{
			totalPackSlotsRequired++;
		}
		else {
			totalSlotsRequired += (desiredItem->amount / maxStackSize);
			if (desiredItem->amount % maxStackSize > 0)
				totalSlotsRequired++;
		}
		

		if (vendorItem->weenie->InqIntQuality(ITEM_TYPE_INT, TYPE_UNDEF) == TYPE_PROMISSORY_NOTE)
			totalCost += (uint32_t)round((vendorItem->weenie->GetValue() * 1.15) * desiredItem->amount);
		else
			totalCost += (uint32_t)round((vendorItem->weenie->GetValue() * profile.sell_price) * desiredItem->amount);

	}

	if (totalCost >= MAX_COIN_PURCHASE)
		return WERROR_NO_OBJECT; // limit to purchases less than 2 billion pyreal
	
	if (currencyamount < totalCost)
	{		
		buyer->SendText("You don't have enough money.", LTT_DEFAULT);
		return WERROR_NO_OBJECT;
	}

	if (buyer->Container_GetNumFreeMainPackSlots() < totalSlotsRequired)
	{
		buyer->SendText("There's not enough room for the items in your inventory.", LTT_DEFAULT);
		return WERROR_NO_OBJECT;
	}

	if ((buyer->GetContainersCapacity() - buyer->m_Packs.size()) < totalPackSlotsRequired)
	{
		buyer->SendText("There's not pack slots for the items in your inventory.", LTT_DEFAULT);
		return WERROR_NO_OBJECT;
	}

	uint32_t coinConsumed = 0;

	coinConsumed = buyer->ConsumeCoin(totalCost, currencyid);

	if (coinConsumed < totalCost)
	{
		//This shouldn't happen.
		buyer->SendText("Couldn't find all the money for the payment.", LTT_DEFAULT);
		buyer->SpawnInContainer(currencyid, coinConsumed); //give back what we consumed and abort.
		
		return WERROR_NO_OBJECT;
	}

	buyer->EmitSound(Sound_PickUpItem, 1.0f);
	for (auto desiredItem : desiredItems)
	{
		CVendorItem* vendorItem = FindVendorItem(desiredItem->iid);
		CWeenieObject *originalWeenie = vendorItem->weenie;

		if (originalWeenie->m_Qualities.GetInt(ITEM_TYPE_INT, 0) == TYPE_SERVICE)
		{
			DoForcedMotion(Motion_CastSpell);
			MakeSpellcastingManager()->CastSpellInstant(buyer->GetID(), originalWeenie->m_Qualities.GetDID(SPELL_DID, 0));
			DoForcedMotion(Motion_Ready);
		}
		else
		{
			if(vendorItem->isPlayerSold) {
				ResellItemToPlayer(buyer, desiredItem->iid, desiredItem->amount);
			} else {
				buyer->SpawnCloneInContainer(originalWeenie, desiredItem->amount);
			}
		}
	}

	queuedThanks.push_back(buyer->GetID());
	DoVendorEmote(Buy_VendorTypeEmote, buyer->GetID());

	if (profile.trade_id) {
		profile.trade_num = currencyamount;
	} else {
		profile.trade_num = 0;
	}
	SendVendorInventory(buyer);

	return WERROR_NONE;
}

int CVendor::TryBuyItemsFromPlayer(CPlayerWeenie *seller, const std::list<ItemProfile *> &desiredItems)
{
	const uint32_t MAX_COIN_PURCHASE = 1000000000; // limit to purchases less than 1 billion pyreal
	const uint32_t MAX_COIN_ALLOWED = 2000000000; // limit to max amount of coin to less than 2 billion pyreal

	// using WERROR_NO_OBJECT as a generic error, change later if it matters
	if (desiredItems.size() >= 100)
		return WERROR_NO_OBJECT;

	// check price of items
	UINT64 totalValue = 0;

	const std::list<ItemProfile *> filteredItems = GetFilteredItems(desiredItems, seller);
	for (auto desiredItem : filteredItems)
	{
		CWeenieObject *sellerItem = seller->FindContainedItem(desiredItem->iid);
		if (!sellerItem)
			return WERROR_NO_OBJECT;
		if (sellerItem->InqIntQuality(STACK_SIZE_INT, 1) < desiredItem->amount)
			return WERROR_NO_OBJECT;
		if (sellerItem->HasContainerContents()) // can't buy containers that have items in them
			return WERROR_NO_OBJECT;
		if (desiredItem->amount <= 0)
			continue;

		if(sellerItem->InqIntQuality(ITEM_TYPE_INT, TYPE_UNDEF) == TYPE_PROMISSORY_NOTE)
			totalValue += (uint32_t)sellerItem->InqIntQuality(VALUE_INT, 0);
		else
			totalValue += (uint32_t)(sellerItem->InqIntQuality(VALUE_INT, 0) * profile.buy_price);
	}

	if (totalValue >= MAX_COIN_PURCHASE)
	{
		seller->SendText("That purchase is too large.", LTT_DEFAULT);
		return WERROR_NO_OBJECT; // purchase too large
	}

	//check of we have enough room for the coins.
	int maxStackSize;
	int totalSlotsRequired = 1;
	CWeenieDefaults *weenieDefs = g_pWeenieFactory->GetWeenieDefaults(W_COINSTACK_CLASS);
	if (weenieDefs->m_Qualities.InqInt(MAX_STACK_SIZE_INT, maxStackSize))
	{
		if (maxStackSize < 1)
			maxStackSize = 1;
		totalSlotsRequired = max((int)(totalValue / maxStackSize), 1);
	}

	//if (seller->Container_GetNumFreeMainPackSlots() < max(totalSlotsRequired - (int)desiredItems.size(), 1)) -- This doesn't take in to account that the items are coming out of side packs, causes error when attempting to spawn pyreals into pack. Eats all items.
	if (seller->Container_GetNumFreeMainPackSlots() < totalSlotsRequired)
	{
		seller->SendText("There's not enough room for the coins in your inventory.", LTT_DEFAULT);
		return WERROR_NO_OBJECT;
	}

	if (seller->InqIntQuality(COIN_VALUE_INT, 0) + totalValue > MAX_COIN_ALLOWED)
		return WERROR_NO_OBJECT; // user will have too much wealth

	seller->EmitSound(Sound_PickUpItem, 1.0f);

	// take away the items purchased...
	for (auto desiredItem : filteredItems)
	{
		//The client won't allow selling partial stacks
		//In fact it will request an item split in inventory before adding partial stacks and once in the sell list 
		//will give the user an error message stating you can't sell partial stacks. So this makes it easier for us.

		CWeenieObject *weenie = seller->FindContainedItem(desiredItem->iid);
		if (!weenie || weenie->m_Qualities.GetID() == 0)
			continue;

		//If selling an item being wielded, unequip it first.
		if(weenie->IsWielded()) {
			seller->FinishMoveItemToContainer(weenie, seller, 0);
		}

		seller->ReleaseContainedItemRecursive(weenie);
		seller->NotifyContainedItemRemoved(weenie->GetID(), false);
		AddPlayerSoldItem(weenie, desiredItem->amount);
	}

	if(filteredItems.size()>0) {
		queuedThanks.push_back(seller->GetID());
		DoVendorEmote(Sell_VendorTypeEmote, seller->GetID());
	}

	//and add the coins.
	seller->SpawnInContainer(W_COINSTACK_CLASS, totalValue);

	if (filteredItems.size() < desiredItems.size())
		seller->NotifyWeenieErrorWithString(WERROR_TRADE_AI_DOESNT_WANT, GetName().c_str());

	return WERROR_NONE;
}

const std::list<ItemProfile *> CVendor::GetFilteredItems(std::list<ItemProfile *> desiredItems, CPlayerWeenie *seller)
{
	for (std::list<ItemProfile *>::iterator i = desiredItems.begin(); i != desiredItems.end();)
	{
		auto desiredItem = *i;
		CWeenieObject *sellerItem = seller->FindContainedItem(desiredItem->iid);
		if (!sellerItem)
		{
			i = desiredItems.erase(i);
			continue;
		}	

		auto sellerItemValue = sellerItem->InqIntQuality(STACK_SIZE_INT, TRUE) > 1 ? sellerItem->InqIntQuality(STACK_UNIT_VALUE_INT, TRUE) : sellerItem->GetValue();
		auto sellerItemType = sellerItem->InqIntQuality(ITEM_TYPE_INT, 0, TRUE);
		auto sellerItemHasMana = sellerItem->InqIntQuality(ITEM_MAX_MANA_INT, 0, TRUE);

		if (sellerItemType != TYPE_PROMISSORY_NOTE && 
			(profile.min_value > sellerItemValue ||
			profile.max_value < sellerItemValue ||
			!(profile.item_types & sellerItemType) ||
			(!profile.magic && sellerItemHasMana) ||
			(profile.trade_id && profile.trade_id != sellerItem->m_Qualities.id)))
		{
			i = desiredItems.erase(i);
		}
		else
			++i;
	}
	return desiredItems;
}

void CVendor::PreSpawnCreate()
{
	CMonsterWeenie::PreSpawnCreate();

	profile.min_value = InqIntQuality(MERCHANDISE_MIN_VALUE_INT, 0, TRUE);
	profile.max_value = InqIntQuality(MERCHANDISE_MAX_VALUE_INT, 0, TRUE);
	profile.magic = InqBoolQuality(DEAL_MAGICAL_ITEMS_BOOL, FALSE);
	profile.item_types = InqIntQuality(MERCHANDISE_ITEM_TYPES_INT, 0, TRUE);
	profile.buy_price = InqFloatQuality(BUY_PRICE_FLOAT, 0, TRUE);
	profile.sell_price = InqFloatQuality(SELL_PRICE_FLOAT, 0, TRUE);
	profile.trade_id = InqDIDQuality(ALTERNATE_CURRENCY_DID, 0);

	if (m_Qualities._create_list)
	{
		for (auto i = m_Qualities._create_list->begin(); i != m_Qualities._create_list->end(); ++i)
		{
			if (i->destination == DestinationType::Shop_DestinationType || i->destination == DestinationType::Contain_DestinationType)
			{
				if (i->destination == DestinationType::Contain_DestinationType || i->destination == DestinationType::Wield_DestinationType)
				{
					if (i->wcid == profile.trade_id)
					{
						CWeenieDefaults *currency = g_pWeenieFactory->GetWeenieDefaults(i->wcid);
						profile.trade_name = currency->m_Qualities.GetString(NAME_STRING, "");
					}
				}
				else
					AddVendorItem(i->wcid, i->palette, i->shade, -1);
			}
		}
	}
}

void CVendor::SendVendorInventory(CWeenieObject *other)
{
	BinaryWriter vendorInfo;
	vendorInfo.Write<uint32_t>(0x62);
	vendorInfo.Write<uint32_t>(GetID());

	profile.Pack(&vendorInfo);
	
	vendorInfo.Write<uint32_t>((uint32_t)m_Items.size());
	for (auto item : m_Items)
	{
		ItemProfile itemProfile;
		itemProfile.amount = item->amount;
		itemProfile.iid = item->weenie->GetID(); // for now, use the WCID plus arbitrary number to reference this item
		itemProfile.pwd = PublicWeenieDesc::CreateFromQualities(&item->weenie->m_Qualities);
		itemProfile.pwd->_containerID = GetID();
		itemProfile.Pack(&vendorInfo);
	}

	other->SendNetMessage(&vendorInfo, PRIVATE_MSG, TRUE, FALSE);
}

void CVendor::DoVendorEmote(VendorTypeEmote type, uint32_t target_id)
{
	if (m_Qualities._emote_table)
	{
		PackableList<EmoteSet> *emoteSetList = m_Qualities._emote_table->_emote_table.lookup(Vendor_EmoteCategory);

		if (emoteSetList)
		{
			float dice = Random::GenFloat(0.0f, 1.0f);

			for (auto &emoteSet : *emoteSetList)
			{
				if (emoteSet.vendorType != (unsigned int)type)
					continue;

				if (dice < emoteSet.probability)
				{
					MakeEmoteManager()->ExecuteEmoteSet(emoteSet, target_id);

					break;
				}

				dice -= emoteSet.probability;
			}
		}
	}
}

int CVendor::DoUseResponse(CWeenieObject *player)
{
	const uint32_t playerId = player->GetID();

	//If this is the first time, then turn to the player and do a little gesture.
	if(m_ActiveBuyers.find(playerId) == m_ActiveBuyers.end()) {
		queuedThanks.push_back(playerId);
		m_ActiveBuyers.insert(playerId);
	}

	if (profile.trade_id) {
		profile.trade_num = player->RecalculateCoinAmount(profile.trade_id);
	} else {
		profile.trade_num = 0;
		player->RecalculateCoinAmount(W_COINSTACK_CLASS);
	}
		
	SendVendorInventory(player);
	DoVendorEmote(Open_VendorTypeEmote, playerId);
	

	return CWeenieObject::DoUseResponse(player);
}

void CAvatarVendor::PreSpawnCreate()
{
	CVendor::PreSpawnCreate();

	for (uint32_t i = 0; i < g_pWeenieFactory->m_NumAvatars; i++)
	{
		AddVendorItem(g_pWeenieFactory->m_FirstAvatarWCID + i, -1);
	}
}

void CVendor::CheckRange()
{
	double UseDist = InqFloatQuality(USE_RADIUS_FLOAT, 0, TRUE);
	for (std::set<uint32_t>::iterator i = m_ActiveBuyers.begin(); i != m_ActiveBuyers.end();)
	{
		CWeenieObject *player = g_pWorld->FindPlayer(*i);
		if(player)
		{
			if (DistanceTo(player, true) > UseDist)
			{
				DoVendorEmote(Close_VendorTypeEmote, player->GetID());
				i = m_ActiveBuyers.erase(i);
			}
			else
				++i;
		}
		else
			i = m_ActiveBuyers.erase(i);
	}
}

bool CVendor::MergeWithExistingInventory(CWeenieObject* weenie, uint32_t amount) {
	const uint32_t wcid = weenie->m_Qualities.GetID();
	int dummyMaterial;
	int usesLeft;

	//Items that have material types are probably not mergeable.
	if(weenie->m_Qualities.InqInt(MATERIAL_TYPE_INT, dummyMaterial, FALSE)) {
		return false;
	}

	usesLeft = weenie->InqIntQuality(STRUCTURE_INT, -1);


	for(auto item : m_Items) {
		CWeenieObject* vendorItemWeenie = item->weenie;
		if(vendorItemWeenie->m_Qualities.GetID() != wcid) {
			continue;
		}

		//Don't merge partially used items unless they have the same number of usages.
		if(usesLeft != -1 && usesLeft != vendorItemWeenie->InqIntQuality(STRUCTURE_INT, -1))
			continue;

		//Vendor sells unlimited number, so "merge" just means delete.
		if(item->amount != -1) {
			item->amount += amount;
		}
		return true;
	}
	return false;
}

void CVendor::PurgePlayerSoldItems() {
	for(auto iter = m_Items.begin(); iter != m_Items.end(); ) {
		CVendorItem* vendorItem = *iter;
		if(vendorItem->isPlayerSold) {
			iter = m_Items.erase(iter);
		} else {
			++iter;
		}
	}
}