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

	string adjectives[] = { "Destroyer", "Defiler", "Corruptor", "Vanquisher", "Insidious", "Abhorred", "Horrible", "Terrible", "Dominator", "Atrocious",
		"Repulsive", "Vile", "Horrid", "Heinous", "Unholy", "Fearful", "Loathsome", "Terrifying", "Intimidating", "Appalling" };
	int count = 20;

	string name = cName;
	name += " the " + adjectives[rand()%count];
	return name;
}

CItem *CRandomizedFactory::GetRandomItem( int iSlot, int iLevel )
{
	if ( iLevel < 1 ) iLevel = 1;

	string name = GetRandomItemName( iSlot );
	int iStat = rand()%3;
	int iBonus = rand()%(iLevel * 5) + 1;
	int iValue = iBonus * 10;

	CItem *pItem = new CItem( name, iSlot, iStat, iBonus, iValue );
	return pItem;
}

CCombatant *CRandomizedFactory::GetRandomMonster( int iLevel )
{
	return NULL;
}
