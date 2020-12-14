#include <StdAfx.h>
#include "RecipeFactory.h"
#include "InferredPortalData.h"

RecipeFactory::RecipeFactory()
{
}


RecipeFactory::~RecipeFactory()
{
}

void RecipeFactory::Reset()
{
}

void RecipeFactory::Initialize()
{
	CStopWatch watch;

	WINLOG(Data, Normal, "Loading recipes...\n");
	SERVER_INFO << "Loading recipes...";

	json precursorData;
	std::ifstream rpcStream(g_pGlobals->GetGameFile("Data/json/recipeprecursors.json"));
	if (rpcStream.is_open())
	{

		rpcStream >> precursorData;
		rpcStream.close();

	}

	json recipeData;
	std::ifstream rcStream(g_pGlobals->GetGameFile("Data/json/recipes.json"));
	if (rcStream.is_open())
	{
		rcStream >> recipeData;
		rcStream.close();
	}

	precursor_list_t jsonPrecursors;
	recipe_list_t jsonRecipes;

	if(precursorData.size() > 0)
		jsonPrecursors.UnPackJson(precursorData);

	if (recipeData.size() > 0)
		jsonRecipes.UnPackJson(recipeData);

	// load individual file overrides
	LoadOverrides(jsonPrecursors, jsonRecipes);

	if(jsonPrecursors.size() > 0)
		UpdateCraftTableData(jsonPrecursors);

	if (jsonRecipes.size() > 0)
		UpdateExitingRecipes(jsonRecipes);

	double elapsed = watch.GetElapsed();

	WINLOG(Data, Normal, "Finished loading recipes in %fs\n", elapsed);
	SERVER_INFO << "Finished loading recipes in " << elapsed;
}

void RecipeFactory::UpdateCraftTableData(precursor_list_t &precursors)
{
	// for each precursor see if it exists in the table
	for (auto pc : precursors)
	{
		uint64_t toolTarget = ((uint64_t)pc.Tool << 32) | pc.Target;
		g_pPortalDataEx->_craftTableData._precursorMap[toolTarget] = pc.RecipeID;
	}
}

void RecipeFactory::UpdateExitingRecipes(recipe_list_t &recipes)
{
	for (auto pc : recipes)
	{
		uint32_t recipeIdToUpdate = pc._recipeID;
		g_pPortalDataEx->_craftTableData._operations[recipeIdToUpdate] = GetCraftOpertionFromNewRecipe(&pc);
	}
}

CCraftOperation RecipeFactory::GetCraftOpertionFromNewRecipe(JsonCraftOperation * recipe)
{
	CCraftOperation co;
	co._dataID = recipe->_dataID;
	co._difficulty = recipe->_difficulty;
	co._failAmount = recipe->_failAmount;
	co._failMessage = recipe->_failMessage;
	co._failureConsumeTargetAmount = recipe->_failureConsumeTargetAmount;
	co._failureConsumeTargetChance = recipe->_failureConsumeTargetChance;
	co._failureConsumeTargetMessage = recipe->_failureConsumeTargetMessage;
	co._failureConsumeToolAmount = recipe->_failureConsumeToolAmount;
	co._failureConsumeToolChance = recipe->_failureConsumeToolChance;
	co._failureConsumeToolMessage = recipe->_failureConsumeToolMessage;
	co._failWcid = recipe->_failWcid;
	for (int m = 0; m < 8; m++)
	{
		co._mods[m] = recipe->_mods[m];
	}
	for (int r = 0; r < 3; r++)
	{
		co._requirements[r] = recipe->_requirements[r];
	}
	co._skill = recipe->_skill;
	co._SkillCheckFormulaType = recipe->_SkillCheckFormulaType;
	co._successAmount = recipe->_successAmount;
	co._successConsumeTargetAmount = recipe->_successConsumeTargetAmount;
	co._successConsumeTargetChance = recipe->_successConsumeTargetChance;
	co._successConsumeTargetMessage = recipe->_successConsumeTargetMessage;
	co._successConsumeToolAmount = recipe->_successConsumeToolAmount;
	co._successConsumeToolChance = recipe->_successConsumeToolChance;
	co._successConsumeToolMessage = recipe->_successConsumeToolMessage;
	co._successMessage = recipe->_successMessage;
	co._successWcid = recipe->_successWcid;

	return co;
}

void RecipeFactory::LoadOverrides(precursor_list_t &precursors, recipe_list_t &recipes)
{
	std::mutex recipeLock;
	std::mutex precursorLock;
	fs::path root = g_pGlobals->GetGameData("Data", "json");
	PerformLoad(root / "recipes", [&](fs::path path)
	{
		std::ifstream fs(path);

		json jsonData;
		bool parsed = false;

		//uint32_t blockId = 0;
		try
		{
			fs >> jsonData;

			uint32_t recipeId = 0;

			json::const_iterator end = jsonData.end();
			json::const_iterator key = jsonData.find("key");
			json::const_iterator val = jsonData.find("recipe");
			json::const_iterator pre = jsonData.find("precursors");

			if (key != end)
			{
				recipeId = *key;

				if (val != end)
				{
					JsonCraftOperation op;
					if (op.UnPackJson(*val))
					{
						std::scoped_lock lock(recipeLock);
						recipes.push_back(op);
					}
				}

				if (pre != end && pre->is_array())
				{
					for (auto pc : *pre)
					{
						json::const_iterator tool = pc.find("tool");
						json::const_iterator target = pc.find("target");
						if (tool != pc.end() && target != pc.end())
						{
							std::scoped_lock lock(precursorLock);
							CraftPrecursor precursor(recipeId, *tool, *target);
							precursors.push_back(precursor);
						}
					}
				}
			}
		}
		catch (std::exception &ex)
		{
			LOG_PRIVATE(Data, Error, "Failed to parse recipe file %s\n", path.string().c_str());
		}

		fs.close();
	});
}
