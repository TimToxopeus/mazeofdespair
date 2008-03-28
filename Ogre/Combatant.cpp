#include "Combatant.h"

#include "stdlib.h"

CCombatant::CCombatant()
{
	m_iGold = 10;
	m_iAttackPower = 5;
	m_iDefense = 5;
	m_iCurHP = m_iMaxHP = 10;
}

int CCombatant::Attack( CCombatant *pVictim )
{
	int diff = m_iAttackPower - pVictim->GetDef();
	int hit = 35 + diff;
	int roll = rand()%100 + 1;
	if ( roll < hit )
	{
		int damage = 1;
		if ( diff > 0 )
			damage += diff;
		pVictim->TakeDamage( damage );

		return damage;
	}

	return 0;
}

int CCombatant::TakeDamage( int iDamage )
{
	if ( m_iCurHP - iDamage < 0 )
		m_iCurHP = 0;
	else
		m_iCurHP -= iDamage;
	return m_iCurHP;
}
