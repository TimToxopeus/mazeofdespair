#pragma once

#include "Item.h"

class CCombatant
{
protected:
	int m_iGold;
	int m_iAttackPower;
	int m_iDefense;
	int m_iCurHP;
	int m_iMaxHP;

public:
	CCombatant();

	int Attack( CCombatant *pVictim );
	int TakeDamage( int iDamage );

	int GetGold() { return m_iGold; }
	int GetCurHP() { return m_iCurHP; }
	virtual int GetMaxHP() { return m_iMaxHP; }
	virtual int GetATP() { return m_iAttackPower; }
	virtual int GetDef() { return m_iDefense; }
};
