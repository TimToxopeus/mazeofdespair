/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

#include "Item.h"
#include "Combatant.h"
#include "DoubleMarkov.h"

#include <string>

class CRandomizedFactory
{
private:
	CDoubleMarkov *m_pNameGenerator;

	std::string GetRandomItemName( int iSlot );
	std::string GetRandomMonsterName();

public:
	CRandomizedFactory();
	~CRandomizedFactory();

	CItem *GetRandomItem( int iSlot, int iLevel );
	CCombatant *GetRandomMonster( int iLevel );
};
