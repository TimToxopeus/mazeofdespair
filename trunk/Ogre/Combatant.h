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

class CMonster
{
private:
	Entity *m_pEntity;
	SceneNode *m_pNode;
	int m_iTileX, m_iTileY;

public:
	CMonster( Entity *pEntity, SceneNode *pNode, int iTileX, int iTileY );
	bool frameStarted( const FrameEvent &evt );

	int getTileX() { return m_iTileX; }
	int getTileY() { return m_iTileY; }
	Entity *getEntity() { return m_pEntity; }
	SceneNode *getNode() { return m_pNode; }
};