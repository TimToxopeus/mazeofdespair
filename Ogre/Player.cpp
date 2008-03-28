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
	if ( index < 0 || index > 9 )
		return false;

	if ( m_Equipment[m_Inventory[index]->GetSlot()] == NULL )
	{
		m_Equipment[m_Inventory[index]->GetSlot()] = m_Inventory[index];
		m_Inventory[index] = NULL;
		return true;
	}

	return false;
}

bool CPlayer::DequipItemFromEquipment( int slot )
{
	if ( slot < 0 || slot > 9 )
		return false;

	for ( int i = 0; i<10; i++ )
	{
		if ( m_Inventory[i] == NULL )
		{
			m_Inventory[i] = m_Equipment[slot];
			m_Equipment[slot] = NULL;
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

void CPlayer::SellItemFromEquipment( int slot )
{
	if ( slot < 0 || slot > 9 )
		return;

	m_iGold += m_Equipment[slot]->GetValue();
	delete m_Equipment[slot];
	m_Equipment[slot] = NULL;
}

void CPlayer::SellItemFromInventory( int index )
{
	if ( index < 0 || index > 9 )
		return;

	m_iGold += m_Inventory[index]->GetValue();
	delete m_Inventory[index];
	m_Inventory[index] = NULL;
}