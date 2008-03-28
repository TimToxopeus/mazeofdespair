#pragma once

#include "Item.h"
#include "Combatant.h"

class CRandomizedFactory
{
public:
	CItem *GetRandomItem( int iSlot, int iLevel );
	CCombatant *GetRandomMonster( int iLevel );
};
