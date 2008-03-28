#pragma once

#include "Combatant.h"
#include <vector>

class CPlayer : public CCombatant
{
protected:
	CItem *m_Equipment[10];
	CItem *m_Inventory[10];

public:
	CPlayer();

	virtual int GetATP();
	virtual int GetDef();
	virtual int GetMaxHP();

	bool AddItemToInventory( CItem *item );
	bool EquipItemFromInventory( int index );
	bool DequipItemFromEquipment( int slot );
	std::vector<CItem *> GetEquipment();
	std::vector<CItem *> GetInventory();

	void SellItemFromEquipment( int slot );
	void SellItemFromInventory( int index );
};
