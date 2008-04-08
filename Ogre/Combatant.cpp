#include "Combatant.h"

#include "stdlib.h"

CCombatant::CCombatant()
{
	m_szName = "Generic simple creature";
	m_iGold = 10;
	m_iAttackPower = 5;
	m_iDefense = 5;
	m_iCurHP = m_iMaxHP = 10;
}

CCombatant::CCombatant(std::string szName, int iGold, int iATP, int iDef, int iHP )
{
	m_szName = szName;
	m_iGold = iGold;
	m_iAttackPower = iATP;
	m_iDefense = iDef;
	m_iCurHP = m_iMaxHP = iHP;
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


CMonster::CMonster(Entity *pEntity, SceneNode *pNode, int iTileX, int iTileY )
{
	m_pEntity = pEntity;
	m_pNode = pNode;
	m_iTileX = iTileX;
	m_iTileY = iTileY;
}

bool CMonster::frameStarted(const FrameEvent &evt)
{
	m_pNode->yaw( Radian(0.003f) );
	return true;
}

void CMonster::BeginAttackMode()
{

}