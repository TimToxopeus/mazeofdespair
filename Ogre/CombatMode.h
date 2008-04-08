#pragma once

#include "Combatant.h"
#include "Player.h"
#include "GameEngine.h"

class CombatMode
{

private:
	CPlayer * m_pPlayer;
	CCombatant * m_pCombatant;
	CMonster *m_pMonster;
	bool m_bPlayersTurn;

public:
	CombatMode(CPlayer *m_pPlayer, CCombatant *pActualMonster, CMonster *pMonster);
	~CombatMode();

	void CanContinue();
	
	bool Attack( const CEGUI::EventArgs &e );
	bool Item( const CEGUI::EventArgs &e );
	bool Flee( const CEGUI::EventArgs &e );
	bool Hit( const CEGUI::EventArgs &e );
	bool Thunder( const CEGUI::EventArgs &e );
	bool Double( const CEGUI::EventArgs &e );
	bool Back( const CEGUI::EventArgs &e );

};
