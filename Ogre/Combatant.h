#pragma once

#include "Item.h"
#include <string>

class CCombatant
{
protected:
	std::string m_szName;
	int m_iGold;
	int m_iAttackPower;
	int m_iDefense;
	int m_iCurHP;
	int m_iMaxHP;

public:
	CCombatant();
	CCombatant(std::string szName, int iGold, int iATP, int iDef, int iHP );

	int Attack( CCombatant *pVictim );
	int TakeDamage( int iDamage );

	std::string GetName() { return m_szName; }
	int GetGold() { return m_iGold; }
	int GetCurHP() { return m_iCurHP; }
	virtual int GetMaxHP() { return m_iMaxHP; }
	virtual int GetATP() { return m_iAttackPower; }
	virtual int GetDef() { return m_iDefense; }
};
