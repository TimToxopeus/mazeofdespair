#pragma once

#include "Combatant.h"
#include "Player.h"

class CombatMode
{

public:
	CombatMode();
	~CombatMode();

	void StartCombat(CPlayer * pCPlayer, CCombatant * pCCombatant);

};
