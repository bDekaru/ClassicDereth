
#pragma once

class PString
{
public:
    PString();
    PString(const char* szString);
    virtual ~PString();

    void Destroy();

    BOOL UnPack(BYTE** ppData, ULONG iSize);

    inline operator const char *() {
        return m_str.c_str();
    }

	static DWORD compute_hash(const char *str);

    // char *m_szString;
    // DWORD m_Length;
	std::string m_str;
};