
#pragma once

template<typename TStatType, typename TDataType>
class TYPEMod : public PackObj, public PackableJson
{
public:
	using weenie_ptr = CWeenieObject * ;
	using stat_list = PackableHashTableWithJson<TStatType, TDataType>;
	using stat_fn_t = stat_list * (*)(weenie_ptr);

	virtual void Pack(class BinaryWriter *pWriter) override;
	virtual bool UnPack(class BinaryReader *pReader) override;
	virtual void PackJson(json& writer) override;
	virtual bool UnPackJson(const json& reader) override;

	TDataType* query(weenie_ptr obj) { return query(obj, _stat); }
	TDataType* query(weenie_ptr obj, TStatType stat) { return nullptr; }
	stat_list* ensure(weenie_ptr obj) { return nullptr; }

	virtual bool apply(weenie_ptr applyTo, weenie_ptr target, weenie_ptr result, int roll, bool &updateObjDesc)
	{
		// applyTo is the object to be modified here (craft target, craft tool, craft result, actor)
		// target is the craft target
		// result is the craft result

		if (roll < _unk) return false;

		TDataType *val = query(applyTo);
		TDataType *tval = query(target);
		TDataType *rval = query(result);

		shouldUpdateObject(updateObjDesc);

		switch (_operationType)
		{
		case 1: //=
			if (val) *val = _value;
			else ensure(applyTo)->add(_stat, &_value);
			return true;
		case 2: //+
			if (val) *val = *val + _value;
			else ensure(applyTo)->add(_stat, &_value);
			return true;
		case 3: // applyTo -> target
			return transfer(val, tval, applyTo, target);
		case 4: // applyTo -> result
			return transfer(val, rval, applyTo, result);
			// 5, 6?
		case 7: //add spell
			applyTo->m_Qualities.AddSpell((uint32_t)_stat);
			return true;

		default:
			break;
		}

		return false;
	}

	void shouldUpdateObject(bool& update) { }

	bool transfer(TDataType *val, TDataType *tval, weenie_ptr source, weenie_ptr target)
	{
		// transfers should only apply to the tool & actor
		if (val)
		{
			if (tval) *tval = *val;
			else ensure(target)->add(_stat, val);
		}
		return true;
	}

	// dye pot recipes imply that this should be the min-roll to succeed
	int _unk;
	int _operationType;
	TStatType _stat;
	TDataType _value;
};

// specializations
template<>
inline int* TYPEMod<STypeInt, int>::query(weenie_ptr obj, STypeInt stat)
{
	if (obj && obj->m_Qualities.m_IntStats)
		return obj->m_Qualities.m_IntStats->lookup(stat);
	return nullptr;
}
template<>
inline TYPEMod<STypeInt, int>::stat_list* TYPEMod<STypeInt, int>::ensure(weenie_ptr obj)
{
	if (!obj)
		return nullptr;

	if (!obj->m_Qualities.m_IntStats)
		obj->m_Qualities.m_IntStats = new PackableHashTableWithJson<STypeInt, int>();

	return obj->m_Qualities.m_IntStats;
}

template<>
inline double* TYPEMod<STypeFloat, double>::query(weenie_ptr obj, STypeFloat stat)
{
	if (obj && obj->m_Qualities.m_FloatStats)
		return obj->m_Qualities.m_FloatStats->lookup(stat);
	return nullptr;
}
template<>
inline TYPEMod<STypeFloat, double>::stat_list* TYPEMod<STypeFloat, double>::ensure(weenie_ptr obj)
{
	if (!obj)
		return nullptr;

	if (!obj->m_Qualities.m_FloatStats)
		obj->m_Qualities.m_FloatStats = new PackableHashTableWithJson<STypeFloat, double>();

	return obj->m_Qualities.m_FloatStats;
}

template<>
inline uint32_t* TYPEMod<STypeDID, uint32_t>::query(weenie_ptr obj, STypeDID stat)
{
	if (obj && obj->m_Qualities.m_DIDStats)
		return obj->m_Qualities.m_DIDStats->lookup(stat);
	return nullptr;
}
template<>
inline TYPEMod<STypeDID, uint32_t>::stat_list* TYPEMod<STypeDID, uint32_t>::ensure(weenie_ptr obj)
{
	if (!obj)
		return nullptr;

	if (!obj->m_Qualities.m_DIDStats)
		obj->m_Qualities.m_DIDStats = new PackableHashTableWithJson<STypeDID, uint32_t>();

	return obj->m_Qualities.m_DIDStats;
}

template<>
inline uint32_t* TYPEMod<STypeIID, uint32_t>::query(weenie_ptr obj, STypeIID stat)
{
	if (obj && obj->m_Qualities.m_IIDStats)
		return obj->m_Qualities.m_IIDStats->lookup(stat);
	return nullptr;
}
template<>
inline TYPEMod<STypeIID, uint32_t>::stat_list* TYPEMod<STypeIID, uint32_t>::ensure(weenie_ptr obj)
{
	if (!obj)
		return nullptr;

	if (!obj->m_Qualities.m_IIDStats)
		obj->m_Qualities.m_IIDStats = new PackableHashTableWithJson<STypeIID, uint32_t>();

	return obj->m_Qualities.m_IIDStats;
}

template<>
inline BOOL* TYPEMod<STypeBool, BOOL>::query(weenie_ptr obj, STypeBool stat)
{
	if (obj && obj->m_Qualities.m_BoolStats)
		return obj->m_Qualities.m_BoolStats->lookup(stat);
	return nullptr;
}
template<>
inline TYPEMod<STypeBool, BOOL>::stat_list* TYPEMod<STypeBool, BOOL>::ensure(weenie_ptr obj)
{
	if (!obj)
		return nullptr;

	if (!obj->m_Qualities.m_BoolStats)
		obj->m_Qualities.m_BoolStats = new PackableHashTableWithJson<STypeBool, BOOL>();

	return obj->m_Qualities.m_BoolStats;
}

template<>
inline std::string* TYPEMod<STypeString, std::string>::query(weenie_ptr obj, STypeString stat)
{
	if (obj && obj->m_Qualities.m_StringStats)
		return obj->m_Qualities.m_StringStats->lookup(stat);
	return nullptr;
}
template<>
inline TYPEMod<STypeString, std::string>::stat_list* TYPEMod<STypeString, std::string>::ensure(weenie_ptr obj)
{
	if (!obj)
		return nullptr;

	if (!obj->m_Qualities.m_StringStats)
		obj->m_Qualities.m_StringStats = new PackableHashTableWithJson<STypeString, std::string>();

	return obj->m_Qualities.m_StringStats;
}


// TODO: Other prop types
template<>
inline void TYPEMod<STypeInt, int>::shouldUpdateObject(bool& update)
{
	switch (_stat)
	{
		// TODO: Flag all visual properties
	case PALETTE_TEMPLATE_INT:
		update = true;
		break;
	}
}

template<>
inline void TYPEMod<STypeDID, uint32_t>::shouldUpdateObject(bool& update)
{
	switch (_stat)
	{
		// TODO: Flag all visual properties
	case ICON_OVERLAY_DID:
	case ICON_OVERLAY_SECONDARY_DID:
	case ICON_UNDERLAY_DID:
	case ICON_DID:
		update = true;
		break;
	}
}

template<>
inline bool TYPEMod<STypeString, std::string>::transfer(std::string* val, std::string* tval, weenie_ptr source, weenie_ptr target)
{
	switch (_stat)
	{
	case TINKER_NAME_STRING:
	case IMBUER_NAME_STRING:
	case CRAFTSMAN_NAME_STRING:
	{
		// for these we want the source name, not the prop val
		//stat_list* astats = ensure(source);
		//val = astats->lookup(NAME_STRING);
		val = query(source, NAME_STRING);
		break;
	}

	}

	if (val)
	{
		if (tval) *tval = *val;
		else ensure(target)->add(_stat, val);
	}
	return true;
}

template<>
inline bool TYPEMod<STypeIID, uint32_t>::transfer(uint32_t* val, uint32_t* tval, weenie_ptr source, weenie_ptr target)
{
	switch (_stat)
	{
	case ALLOWED_WIELDER_IID:
	case ALLOWED_ACTIVATOR_IID:
	{
		// for these we want the item id, not the prop val
		val = &source->id;
		break;
	}

	}

	if (val)
	{
		if (tval) *tval = *val;
		else ensure(target)->add(_stat, val);
	}
	return true;
}

template<typename TStatType, typename TDataType>
class TYPERequirement : public PackObj, public PackableJson
{
public:
	using quality_table_t = PackableHashTableWithJson<TStatType, TDataType>;

	virtual void Pack(class BinaryWriter *pWriter) override;
	virtual bool UnPack(class BinaryReader *pReader) override;
	virtual void PackJson(json& writer) override;
	virtual bool UnPackJson(const json& reader) override;

	virtual bool pass(quality_table_t *table)
	{
		TDataType *val = nullptr;
		if (table)
		{
			val = table->lookup(_stat);
		}

		switch (_operationType)
		{
		case 1: //> - unconfirmed
			return val && *val > _value;
		case 2: //>=
			return val && *val >= _value;
		case 3: //<
			return !val || *val < _value;
		case 4: //<=
			return !val || *val <= _value;
		case 5: //==
			return val && *val == _value;
		case 6: //!=
			return !val || *val != _value;
		case 7: //exists
			return val != nullptr;
		case 8: //doesnt exist
			return val == nullptr;
			// ??
			//case 9: //exists and != (or maybe just exists)
			//	if (exists && value != intRequirement._value)
			//	{
			//		SendText(intRequirement._message.c_str(), LTT_CRAFT);
			//		return false;
			//	}
			//	break;
		default:
			break;
		}

		return false;
	}

	TStatType _stat;
	TDataType _value;
	int _operationType;
	std::string _message;
};

class CraftRequirements : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON();

	virtual bool pass(CWeenieObject *actor, CWeenieObject *item)
	{
		if (!pass(actor, _intRequirement, item->m_Qualities.m_IntStats))
			return false;

		if (!pass(actor, _didRequirement, item->m_Qualities.m_DIDStats))
			return false;

		if (!pass(actor, _iidRequirement, item->m_Qualities.m_IIDStats))
			return false;

		if (!pass(actor, _floatRequirement, item->m_Qualities.m_FloatStats))
			return false;

		if (!pass(actor, _stringRequirement, item->m_Qualities.m_StringStats))
			return false;

		if (!pass(actor, _boolRequirement, item->m_Qualities.m_BoolStats))
			return false;

		return true;
	}

	template <typename stat_type_t, typename val_type_t>
	bool pass(CWeenieObject *actor, PackableListWithJson<TYPERequirement<stat_type_t, val_type_t>> &reqs, PackableHashTableWithJson<stat_type_t, val_type_t> *stats)
	{
		for (auto req : reqs)
		{
			if (!req.pass(stats))
			{
				actor->SendText(req._message.c_str(), LTT_CRAFT);
				return false;
			}
		}
		return true;
	}

	PackableListWithJson<TYPERequirement<STypeInt, int>> _intRequirement;
	PackableListWithJson<TYPERequirement<STypeDID, uint32_t>> _didRequirement;
	PackableListWithJson<TYPERequirement<STypeIID, uint32_t>> _iidRequirement;
	PackableListWithJson<TYPERequirement<STypeFloat, double>> _floatRequirement;
	PackableListWithJson<TYPERequirement<STypeString, std::string>> _stringRequirement;
	PackableListWithJson<TYPERequirement<STypeBool, BOOL>> _boolRequirement;
};

class CraftMods : public PackObj, public PackableJson
{
public:
	using weenie_ptr = CWeenieObject * ;

	template <typename key_t, typename value_t>
	using type_mod_list = PackableListWithJson<TYPEMod<key_t, value_t>>;

	template <typename key_t, typename value_t>
	using stat_list = PackableHashTableWithJson<key_t, value_t>;

	template <typename key_t, typename value_t>
	using stat_fn_t = stat_list<key_t, value_t>*(*)(weenie_ptr);

	virtual void Pack(class BinaryWriter *pWriter) override;
	virtual bool UnPack(class BinaryReader *pReader) override;
	virtual void PackJson(json& writer) override;
	virtual bool UnPackJson(const json& reader) override;

	virtual bool apply(weenie_ptr applyTo, weenie_ptr target, weenie_ptr result)
	{
		// https://stackoverflow.com/questions/18889028/a-positive-lambda-what-sorcery-is-this
		//#define Q(type) +[](weenie_ptr w) { return w ? w->m_Qualities.m_ ## type ## Stats : nullptr; }
		//#define QInt() Q(Int)
		//#define QDID() Q(DID)
		//#define QIID() Q(IID)
		//#define QFloat() Q(Float)
		//#define QStr() Q(String)
		//#define QBool() Q(Bool)
		// api also needs result item
		// TYPEMod::apply may copy from tool -> target or tool -> result
		// not sure of proper flow
		bool success = true;
		int roll = Random::GenInt(1, 100);

		success &= apply(_intMod, applyTo, target, result, roll);
		success &= apply(_didMod, applyTo, target, result, roll);
		success &= apply(_iidMod, applyTo, target, result, roll);
		success &= apply(_floatMod, applyTo, target, result, roll);
		success &= apply(_stringMod, applyTo, target, result, roll);
		success &= apply(_boolMod, applyTo, target, result, roll);

		return success;
	}

	template <typename key_t, typename value_t>
	bool apply(type_mod_list<key_t, value_t> &mods,
		weenie_ptr applyTo, weenie_ptr target, weenie_ptr result, int roll)
	{
		for (auto mod : mods)
		{
			if (!mod.apply(applyTo, target, result, roll, _unknown7))
			{
				//actor->SendText(req._message.c_str(), LTT_CRAFT);
				return false;
			}
		}
		return true;
	}

	PackableListWithJson<TYPEMod<STypeInt, int>> _intMod;
	PackableListWithJson<TYPEMod<STypeDID, uint32_t>> _didMod;
	PackableListWithJson<TYPEMod<STypeIID, uint32_t>> _iidMod;
	PackableListWithJson<TYPEMod<STypeFloat, double>> _floatMod;
	PackableListWithJson<TYPEMod<STypeString, std::string>> _stringMod;
	PackableListWithJson<TYPEMod<STypeBool, BOOL>> _boolMod;

	int _ModifyHealth = 0;
	int _ModifyStamina = 0;
	int _ModifyMana = 0;
	int _RequiresHealth = 0;
	int _RequiresStamina = 0;
	int _RequiresMana = 0;

	// visuals updated
	bool _unknown7 = false;

	uint32_t _modificationScriptId = 0;
	int _unknown9 = 0;

	uint32_t _unknown10 = 0;
};

class CraftOperationData : public PackableJson
{
public:
	DECLARE_PACKABLE_JSON();

	uint32_t _unk = 0;
	STypeSkill _skill = STypeSkill::UNDEF_SKILL;
	int _difficulty = 0;
	uint32_t _SkillCheckFormulaType = 0;
	uint32_t _successWcid = 0;
	uint32_t _successAmount = 0;
	std::string _successMessage;
	uint32_t _failWcid = 0;
	uint32_t _failAmount = 0;
	std::string _failMessage;

	double _successConsumeTargetChance;
	int _successConsumeTargetAmount;
	std::string _successConsumeTargetMessage;

	double _successConsumeToolChance;
	int _successConsumeToolAmount;
	std::string _successConsumeToolMessage;

	double _failureConsumeTargetChance;
	int _failureConsumeTargetAmount;
	std::string _failureConsumeTargetMessage;

	double _failureConsumeToolChance;
	int _failureConsumeToolAmount;
	std::string _failureConsumeToolMessage;

	CraftRequirements _requirements[3];
	CraftMods _mods[8];

	uint32_t _dataID = 0;
};

class CCraftOperation : public PackObj, public CraftOperationData
{
public:
	DECLARE_PACKABLE()

};

class JsonCraftOperation : public CraftOperationData
{
public:
	JsonCraftOperation() = default;
	JsonCraftOperation(const CraftOperationData& other)
		: CraftOperationData(other), _recipeID()
	{
	}
	JsonCraftOperation(const CraftOperationData& other, uint32_t id)
		: CraftOperationData(other), _recipeID(id)
	{
	}

	DECLARE_PACKABLE_JSON();

	uint32_t _recipeID = 0;
};

class CCraftTable : public PackObj, public PackableJson
{
public:
	CCraftTable();
	virtual ~CCraftTable() override;

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON();


	PackableHashTable<uint32_t, CCraftOperation> _operations;
	PackableHashTable<uint64_t, uint32_t, uint64_t> _precursorMap;
};

class CraftPrecursor : public PackableJson
{
public:
	CraftPrecursor() = default;
	CraftPrecursor(uint32_t recipe, uint32_t tool, uint32_t target) :
		RecipeID(recipe), Tool(tool), Target(target)
	{ }

	DECLARE_PACKABLE_JSON();

	uint32_t Tool = 0;
	uint32_t Target = 0;
	uint32_t RecipeID = 0;
	uint64_t ToolTargetCombo = 0;

};

