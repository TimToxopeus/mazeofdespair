#include "RandomizedFactory.h"

#include <string>
using namespace std;

CRandomizedFactory::CRandomizedFactory()
{
	m_pNameGenerator = new CDoubleMarkov();
}

CRandomizedFactory::~CRandomizedFactory()
{
	delete m_pNameGenerator;
	m_pNameGenerator = NULL;
}

string CRandomizedFactory::GetRandomItemName( int iSlot )
{
	string adjectives[] = { "Benevolence", "Impurity", "Evil", "Light", "Darkness", "Hope", "Energy", "the Bear", "Mobility", "Justice",
		"Infamy", "Savagery", "Ruin", "Fury", "the Ancients", "the Fish", "the Dairy Farmer", "Success", "Eternity", "Fate" };
	int count = 20;
	
	string slots[] = { "Helm", "Chest-Guard", "Leggings", "Gloves", "Boots", "Sword", "Shield", "Ring", "Cloak", "Amulet" };

	string name = "The " + slots[iSlot] + " of " + adjectives[rand()%count];
	return name;
}

string CRandomizedFactory::GetRandomMonsterName()
{
	char cName[12];
	m_pNameGenerator->GenerateName( 10, cName );

	string name = cName;
	return name;
}

CItem *CRandomizedFactory::GetRandomItem( int iSlot, int iLevel )
{
	return NULL;
}

CCombatant *CRandomizedFactory::GetRandomMonster( int iLevel )
{
	return NULL;
}
