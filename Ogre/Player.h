/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

#include "Combatant.h"
#include <vector>

class CPlayer : public CCombatant
{
protected:
	CItem *m_Equipment[10];
	CItem *m_Inventory[10];
	int GetRealIndex( int index, bool equipment );
	int m_iRage;

public:
	CPlayer();

	virtual int GetATP();
	virtual int GetDef();
	virtual int GetMaxHP();
	
	int GetRage();
	void AddRage(int pRage);

	bool AddItemToInventory( CItem *item );
	void DeductCash( int amount ) { m_iGold -= amount; }
	bool EquipItemFromInventory( int index );
	bool DequipItemFromEquipment( int index );
	std::vector<CItem *> GetEquipment();
	std::vector<CItem *> GetInventory();

	void SellItemFromEquipment( int index );
	void SellItemFromInventory( int index );

	int ThunderStrike(CCombatant *pCCombatant);
	int DoubleSlash(CCombatant *pCCombatant);

	void RecoverHP() { m_iCurHP = m_iMaxHP; }
};
