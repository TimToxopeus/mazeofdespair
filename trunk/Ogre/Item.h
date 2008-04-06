#pragma once

#include <string>
#include "Ogre.h"
using namespace Ogre;

// Stat value 0 = Attack power
// Stat value 1 = Defensive power
// Stat value 2 = Maximum hitpoints

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
	inline std::string GetBonusString() { std::string str; if ( m_iStat == 0 ) str = "Attack power"; else if ( m_iStat == 1 ) str = "Defensive power"; else str = "Hitpoints"; return str; }
};

class CLevelItem
{
private:
	Entity *m_pEntity;
	SceneNode *m_pNode;
	int m_iTileX, m_iTileY;

public:
	CLevelItem( Entity *pEntity, SceneNode *pNode, int iTileX, int iTileY );
	bool frameStarted( const FrameEvent &evt );
	SceneNode* getSceneNode();

	int getTileX() { return m_iTileX; }
	int getTileY() { return m_iTileY; }
	Entity *getEntity() { return m_pEntity; }
	SceneNode *getNode() { return m_pNode; }
};
