
#include <StdAfx.h>
#include "Vendor.h"
#include "WeenieFactory.h"
#include "World.h"
#include "Player.h"
#include "EmoteManager.h"
#include "SpellcastingManager.h"

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
}

CVendor::~CVendor()
{
	ResetItems();
}

void CVendor::Tick()
{
	if (m_EmoteManager)
		m_EmoteManager->Tick();

	if (m_VendorCycleTime <= Timer::cur_time)
	{
		CheckRange();
		m_VendorCycleTime = (Timer::cur_time + 3);
	}

}

void CVendor::ResetItems()
{
	for (auto item : m_Items)
	{
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
	for (auto item : m_Items)
	{
		if (item->weenie->GetID() == item_id)
			return item;
	}

	return NULL;
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
	// clone the weenie
	for (auto desiredItem : desiredItems)
	{
		CWeenieObject *originalWeenie = FindVendorItem(desiredItem->iid)->weenie;

		if (originalWeenie->m_Qualities.GetInt(ITEM_TYPE_INT, 0) == TYPE_SERVICE)
		{
			DoForcedMotion(Motion_CastSpell);
			MakeSpellcastingManager()->CastSpellInstant(buyer->GetID(), originalWeenie->m_Qualities.GetDID(SPELL_DID, 0));
			DoForcedMotion(Motion_Ready);
		}
		else
		{
			originalWeenie->SetID(CWorld::GenerateGUID(eDynamicGUID));
			buyer->SpawnCloneInContainer(originalWeenie, desiredItem->amount);
		}
	}

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
			totalValue += (uint32_t)round(sellerItem->InqIntQuality(VALUE_INT, 0));
		else
			totalValue += (uint32_t)round(sellerItem->InqIntQuality(VALUE_INT, 0) * profile.buy_price);
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
		if (!weenie)
			continue;
		weenie->Remove(); //todo: maybe add vendors relisting loot items like they used to way back.
	}
	if (filteredItems.size()>0)
		DoVendorEmote(Sell_VendorTypeEmote, seller->GetID());

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

void CVendor::AddVendorItemByAllMatchingNames(const char *name)
{
	int index = 0;
	uint32_t wcid;
	
	while (wcid = g_pWeenieFactory->GetWCIDByName(name, index++))
	{
		AddVendorItem(wcid, -1);
	}
}

void CVendor::GenerateItems()
{
	ResetItems();
}

void CVendor::GenerateAllItems()
{
	ResetItems();
}

void CVendor::ValidateItems()
{
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
		for (auto i = m_Qualities._create_list->begin(); i != m_Qualities._create_list->end(); i++)
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
	ValidateItems();

	BinaryWriter vendorInfo;
	vendorInfo.Write<uint32_t>(0x62);
	vendorInfo.Write<uint32_t>(GetID());

	profile.Pack(&vendorInfo);
	
	vendorInfo.Write<uint32_t>(m_Items.size());
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

void CVendor::DoVendorEmote(int type, uint32_t target_id)
{
	if (m_Qualities._emote_table)
	{
		PackableList<EmoteSet> *emoteSetList = m_Qualities._emote_table->_emote_table.lookup(Vendor_EmoteCategory);

		if (emoteSetList)
		{
			double dice = Random::GenFloat(0.0, 1.0);

			for (auto &emoteSet : *emoteSetList)
			{
				if (emoteSet.vendorType != type)
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
	if (IsCompletelyIdle())
	{
		MovementParameters params;
		TurnToObject(player->GetID(), &params);
	}

	if (profile.trade_id) {
		profile.trade_num = player->RecalculateCoinAmount(profile.trade_id);
	} else {
		profile.trade_num = 0;
		player->RecalculateCoinAmount(W_COINSTACK_CLASS);
	}
		
	SendVendorInventory(player);
	DoVendorEmote(Open_VendorTypeEmote, player->GetID());
	m_ActiveBuyers.insert(player->GetID());

	return CWeenieObject::DoUseResponse(player);
}

void CAvatarVendor::PreSpawnCreate()
{
	CVendor::PreSpawnCreate();

	// ResetItems();

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
