//#include "CombatMode.h"
#include "GameEngine.h"

extern std::string itoa2( const int x );

CombatMode::CombatMode(CPlayer *pCPlayer, CCombatant *pCCombatant, CMonster *pMonster)
{
	m_pPlayer = pCPlayer;
	m_pCombatant = pCCombatant;
	m_pMonster = pMonster;
	CGameEngine *engine = CGameEngine::Instance();
	engine->SetCameraPosition( Vector3( 8750, 0, 8750 ));
	engine->SetGUIMode( BATTLEMODEMENU );
	pMonster->getNode()->setPosition(engine->GetCameraPosition() + engine->GetCameraDirection() * 150 );
	m_bPlayersTurn = true;
	
	AddStringToCombatLog( "Starting battle with " + pCCombatant->GetName() );
}

CombatMode::~CombatMode()
{
	
}

void CombatMode::AddStringToCombatLog(std::string pText)
{
	m_sCombatText.push_back(pText);
	if ( m_sCombatText.size() > 4 )
	{
		m_sCombatText.erase( m_sCombatText.begin() );
	}
	PrintCombatLog();
}

void CombatMode::PrintCombatLog()
{
	std::string message = "";
	for (int i = 0; i < m_sCombatText.size(); i++ )
	{
		message += m_sCombatText[i] + " \n";
	}
	CGameEngine::Instance()->SetCombatText( message );
}

// Combat Menu
bool CombatMode::Attack( const CEGUI::EventArgs &e )
{
	CGameEngine::Instance()->SetGUIMode( BATTLEMODEATTACK );
	return true;
}

bool CombatMode::Item( const CEGUI::EventArgs &e )
{	
	//CGameEngine::Instance()->SetCombatText( "Item!" );
	return true;
}

bool CombatMode::Flee( const CEGUI::EventArgs &e )
{
	//// Calculate flee chance.
	//int diff = m_iAttackPower - pVictim->GetDef();
	//int hit = 35 + diff;
	//int roll = rand()%100 + 1;
	//if ( roll < hit )
	//{
		

	// Continue game
	CGameEngine::Instance()->ContinueGame(m_pCombatant, m_pMonster);
	
	return true;
}

// Attack Menu
bool CombatMode::Hit( const CEGUI::EventArgs &e )
{
	int damage = m_pPlayer->Attack(m_pCombatant);
	if ( damage > 0 )
	{
		AddStringToCombatLog( "You hit " + m_pCombatant->GetName() + " with " + itoa2(damage) + "!" );
		m_pPlayer->AddRage(2);
	}
	else
	{
		AddStringToCombatLog( "You miss " + m_pCombatant->GetName() + "!" );
	}
	m_bPlayersTurn = false;
	CanContinue();	
	return true;
}

bool CombatMode::Thunder( const CEGUI::EventArgs &e )
{
	//CGameEngine::Instance()->SetCombatText( "THUNDERRR! " );
	m_bPlayersTurn = false;
	return true;
}

bool CombatMode::Double( const CEGUI::EventArgs &e )
{
	//CGameEngine::Instance()->SetCombatText( "DOUBLE SLASH!! " );
	m_bPlayersTurn = false;
	return true;
}

bool CombatMode::Back( const CEGUI::EventArgs &e )
{
	CGameEngine::Instance()->SetGUIMode( BATTLEMODEMENU );
	return true;
}

void CombatMode::CanContinue()
{
	if ( m_pPlayer->GetCurHP() == 0 )
	{
		AddStringToCombatLog( "You die! Game Over " );
		CGameEngine::Instance()->ContinueGame(m_pCombatant, m_pMonster);		
	}
	else if ( m_pCombatant->GetCurHP() == 0 )
	{
		AddStringToCombatLog( "You have slain " + m_pCombatant->GetName() + "!");
		CGameEngine::Instance()->ContinueGame(m_pCombatant, m_pMonster);
	}
	else if ( !m_bPlayersTurn ) 
	{
		int damage = m_pCombatant->Attack(m_pPlayer);
		if ( damage > 0 )
		{
			AddStringToCombatLog( m_pCombatant->GetName()  + " hits you with " + itoa2(damage) + " damage!" );
		}
		else
		{
			AddStringToCombatLog( m_pCombatant->GetName() + " misses you! " );
		}
		m_bPlayersTurn = true;
		
		CanContinue();
	}
}





