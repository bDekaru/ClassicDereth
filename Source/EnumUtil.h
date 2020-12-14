#pragma once



class EnumUtility
{
public:
	EnumUtility();

	CharacterOption StringToCharacterOption(std::string strOption);
	std::string CharacterOptionToString(CharacterOption coOption);

	CharacterOptions2 StringToCharacterOptions2(std::string strOption);
	std::string CharacterOptions2ToString(CharacterOptions2 coOption);
private:
	void loadStringToCharacterOptions();

	std::map<std::string, CharacterOption> m_mapStringToCharacterOption;
	std::map<CharacterOption, std::string> m_mapCharacterOptionToString;

	std::map<std::string, CharacterOptions2> m_mapStringToCharacterOptions2;
	std::map<CharacterOptions2, std::string> m_mapCharacterOptions2ToString;
};

extern EnumUtility EnumUtil;