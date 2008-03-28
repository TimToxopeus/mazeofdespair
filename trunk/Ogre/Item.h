#pragma once

#include <string>

class CItem
{
protected:
	std::string m_szName;
	int m_iSlot;
	int m_iStat;
	int m_iBonus;
	int m_iValue;

public:
	CItem( std::string szName, int iSlot, int iStat, int iBonus, int iValue )
	{
		m_szName = szName;
		m_iSlot = iSlot;
		m_iStat = iStat;
		m_iBonus = iBonus;
		m_iValue = iValue;
	}

	inline std::string GetName() { return m_szName; }
	inline int GetSlot() { return m_iSlot; }
	inline int GetStat() { return m_iStat; }
	inline int GetBonus() { return m_iBonus; }
	inline int GetValue() { return m_iValue; }
};
