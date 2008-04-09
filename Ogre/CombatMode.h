/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

#include "Combatant.h"
#include "Player.h"
#include "GameEngine.h"

class CombatMode
{

private:
	CPlayer * m_pPlayer;						// Player object for combat
	CCombatant * m_pCombatant;					// Combatant object for combat
	CMonster *m_pMonster;						// Monster object for combat
	bool m_bPlayersTurn;						// Boolean that holds current attack turn
	std::vector<std::string> m_sCombatText;		// Vector for combat text. 

public:
	CombatMode(CPlayer *m_pPlayer, CCombatant *pActualMonster, CMonster *pMonster);
	~CombatMode();

	// Function that checks whether the combat can continue or has to terminate.
	void CanContinue();
	
	// Combat event handlers 
	bool Attack( const CEGUI::EventArgs &e );
	bool Flee( const CEGUI::EventArgs &e );
	bool Hit( const CEGUI::EventArgs &e );
	bool Thunder( const CEGUI::EventArgs &e );
	bool Double( const CEGUI::EventArgs &e );
	bool Back( const CEGUI::EventArgs &e );

	
	void PrintCombatLog();							// Function that prints the combat log.
	void AddStringToCombatLog(std::string pstring);	// Function that adds text toe the combat log.

	CCombatant *GetCombatant() { return m_pCombatant; }	
};
