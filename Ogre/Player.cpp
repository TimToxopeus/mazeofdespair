#include "Player.h"

#include <vector>
using namespace std;

CPlayer::CPlayer()
{
	m_szName = "Player";
	m_iGold = 150;
	m_iAttackPower = 10;
	m_iDefense = 10;
	m_iCurHP = m_iMaxHP = 20;
	m_iRage = 0;

	for ( int i = 0; i <10; i++ )
	{
		m_Equipment[i] = NULL;
		m_Inventory[i] = NULL;
	}
}

int CPlayer::GetATP()
{
	int atp = m_iAttackPower;

	for ( int i = 0; i<10; i++ )
	{
		if ( m_Equipment[i] != NULL )
		{
			if ( m_Equipment[i]->GetStat() == 0 )
				atp += m_Equipment[i]->GetBonus();
		}
	}

	return atp;
}

int CPlayer::GetDef()
{
	int def = m_iDefense;

	for ( int i = 0; i<10; i++ )
	{
		if ( m_Equipment[i] != NULL )
		{
			if ( m_Equipment[i]->GetStat() == 1 )
				def += m_Equipment[i]->GetBonus();
		}
	}

	return def;
}

int CPlayer::GetMaxHP()
{
	int max = m_iMaxHP;

	for ( int i = 0; i<10; i++ )
	{
		if ( m_Equipment[i] != NULL )
		{
			if ( m_Equipment[i]->GetStat() == 2 )
				max += m_Equipment[i]->GetBonus();
		}
	}

	return max;
}

bool CPlayer::AddItemToInventory( CItem *item )
{
	for ( int i = 0; i<10; i++ )
	{
		if ( m_Inventory[i] == NULL )
		{
			m_Inventory[i] = item;
			return true;
		}
	}
	return false;
}

bool CPlayer::EquipItemFromInventory( int index )
{
	int a = GetRealIndex( index, false );
	if ( a == -1 )
		return false;
	if ( m_Equipment[m_Inventory[a]->GetSlot()] == NULL )
	{
		m_Equipment[m_Inventory[a]->GetSlot()] = m_Inventory[a];
		m_Inventory[a] = NULL;
		return true;
	}

	return false;
}

bool CPlayer::DequipItemFromEquipment( int index )
{
	int a = GetRealIndex( index, true );
	if ( a == -1 )
		return false;
	for ( int i = 0; i<10; i++ )
	{
		if ( m_Inventory[i] == NULL )
		{
			m_Inventory[i] = m_Equipment[a];
			m_Equipment[a] = NULL;
			return true;
		}
	}

	return false;
}

vector<CItem *> CPlayer::GetEquipment()
{
	vector<CItem *> equipment;

	for ( int i = 0; i<10; i++ )
		if ( m_Equipment[i] )
			equipment.push_back( m_Equipment[i] );

	return equipment;
}

vector<CItem *> CPlayer::GetInventory()
{
	vector<CItem *> inventory;

	for ( int i = 0; i<10; i++ )
		if ( m_Inventory[i] )
			inventory.push_back( m_Inventory[i] );

	return inventory;
}

void CPlayer::SellItemFromEquipment( int index )
{
	int a = GetRealIndex(index, true);
	if ( a != -1 )

	{
		m_iGold += m_Equipment[a]->GetValue();
		delete m_Equipment[a];
		m_Equipment[a] = NULL;
	}
}

void CPlayer::SellItemFromInventory( int index )
{
	int a = GetRealIndex(index, false);
	if ( a != -1 )
	{
		m_iGold += m_Inventory[a]->GetValue();
		delete m_Inventory[a];
		m_Inventory[a] = NULL;
	}
}

int CPlayer::GetRealIndex( int index, bool equipment )
{
	if ( index < 0 || index > 9 )
		return -1;

	int a = 0;
	int i = index;
	if ( i == 0 )
	{
		// set a to the first result
		for ( a = 0; a<10; a++ )
		{
			if ( equipment )
			{
				if ( m_Equipment[a] != NULL )
					break;
			}
			else
			{
				if ( m_Inventory[a] != NULL )
					break;
			}
		}
	}
	else
	{
		i++;
		for ( a = 0; a<10; a++ )
		{
			if ( equipment )
			{
				if ( m_Equipment[a] != NULL )
					i--;
			}
			else
			{
				if ( m_Inventory[a] != NULL )
					i--;
			}
			if ( i == 0 )
				break;
		}
	}

	if ( i == 0 )
		return a;
	return -1;
}

int CPlayer::GetRage()
{
	return m_iRage;
}

void CPlayer::AddRage(int pRage)
{
	m_iRage += pRage;
}

int CPlayer::ThunderStrike(CCombatant *pVictim)
{
	// If the player has enough rage, execute the attack.
	if ( m_iRage > 4 ) 
	{
		// Only subtract 4, since the player gets 1 Rage point for attacking.
		m_iRage -= 4;
		int diff = m_iAttackPower - pVictim->GetDef();
		int hit = 35 + diff;
		int roll = rand()%100 + 1;
		if ( roll < hit )
		{
			int damage = 4;
			if ( diff > 0 )
				damage += diff;
			pVictim->TakeDamage( damage );

			return damage;
		}
	}

	return 0;
}

int CPlayer::DoubleSlash(CCombatant *pVictim)
{
	// If the player has enough rage, execute the attack.
	if ( m_iRage > 7 ) 
	{
		// Only subtract 6, since the player gets 2 Rage points for attacking.
		m_iRage -= 6;
		int diff = m_iAttackPower - pVictim->GetDef();
		int hit = 35 + diff;
		int roll = rand()%100 + 1;
		if ( roll < hit )
		{
			int damage = 6;
			if ( diff > 0 )
				damage += diff;
			pVictim->TakeDamage( damage );

			return damage;
		}
	}

	return 0;
}