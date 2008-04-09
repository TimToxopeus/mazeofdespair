/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#include "GameEngine.h"

extern std::string itoa2( const int x );

CombatMode::CombatMode(CPlayer *pCPlayer, CCombatant *pCCombatant, CMonster *pMonster)
{
	m_pPlayer = pCPlayer;
	m_pCombatant = pCCombatant;
	m_pMonster = pMonster;
	CGameEngine *engine = CGameEngine::Instance();
	
	// Set camera position for combat mode.
	engine->SetCameraPosition( Vector3( 8750, 0, 8750 ));
	// Set Combat GUI Mode.
	engine->SetGUIMode( BATTLEMODEMENU );
	
	// Set Enemy position.
	pMonster->getNode()->setPosition(engine->GetCameraPosition() + engine->GetCameraDirection() * 150 );
	// Set Players turn on.
	m_bPlayersTurn = true;
	
	// Set starting Combat text.
	AddStringToCombatLog( "Starting battle with " + pCCombatant->GetName() );
}

CombatMode::~CombatMode()
{
	
}
/*
 *	Function that sets the combat log. It keeps the log to 4 lines.
 */
void CombatMode::AddStringToCombatLog(std::string pText)
{
	Ogre::LogManager::getSingleton().logMessage( pText );
	m_sCombatText.push_back(pText);
	if ( m_sCombatText.size() > 4 )
	{
		m_sCombatText.erase( m_sCombatText.begin() );
	}
	PrintCombatLog();
}

/*
 *	Function that prints the combat log. It fills the text-vector with strings to print.
 */
void CombatMode::PrintCombatLog()
{
	std::string message = "";
	for (int i = 0; i < m_sCombatText.size(); i++ )
	{
		message += m_sCombatText[i] + " \n";
	}
	CGameEngine::Instance()->SetCombatText( message );
}

bool CombatMode::Attack( const CEGUI::EventArgs &e )
{
	// Set GUI Mode to attackmode
	CGameEngine::Instance()->SetGUIMode( BATTLEMODEATTACK );
	return true;
}

bool CombatMode::Flee( const CEGUI::EventArgs &e )
{	
	if ( m_pCombatant != NULL && m_pMonster != NULL )
	{
		// Continue game	
		CGameEngine::Instance()->ContinueGame(m_pCombatant, m_pMonster);
		// Delete objects to prevent leaks
		delete m_pCombatant;
		m_pCombatant = NULL;
		delete m_pMonster;
		m_pMonster = NULL;
	}

	return true;
}

bool CombatMode::Hit( const CEGUI::EventArgs &e )
{
	// Gives damage to the enemy
	int damage = m_pPlayer->Attack(m_pCombatant);
	if ( damage > 0 )
	{
		// Print combat log
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
	// Checks for enough rage and if so, attack with thunder
	if ( m_pPlayer->GetRage() > 4 ) 
	{
		int damage = m_pPlayer->ThunderStrike(m_pCombatant);
		if ( damage > 0 )
		{
			AddStringToCombatLog( "You ThunderStrike " + m_pCombatant->GetName() + " with " + itoa2(damage) + "!!" );		
		}
		else
		{
			AddStringToCombatLog( "You miss " + m_pCombatant->GetName() + "!" );
		}
		m_bPlayersTurn = false;
		CanContinue();	
	}
	else 
	{
		AddStringToCombatLog( "You don't have enough Rage!\n    Need at least 3 for ThunderStrike!" );
	}

	return true;
}

bool CombatMode::Double( const CEGUI::EventArgs &e )
{
		// Checks for enough rage and if so, attack with double slash
	if ( m_pPlayer->GetRage() > 8 ) 
	{
		int damage = m_pPlayer->DoubleSlash(m_pCombatant);
		if ( damage > 0 )
		{
			AddStringToCombatLog( "You Double Slash " + m_pCombatant->GetName() + " with " + itoa2(damage) + "!!!" );		
		}
		else
		{
			AddStringToCombatLog( "You miss " + m_pCombatant->GetName() + "!" );
		}
		m_bPlayersTurn = false;
		CanContinue();	
	}
	else 
	{
		AddStringToCombatLog( "You don't have enough Rage!\n    Need at least 6 for Double Slash!" );
	}

	return true;
}

bool CombatMode::Back( const CEGUI::EventArgs &e )
{
	// Sets gui mode back to combat mode
	CGameEngine::Instance()->SetGUIMode( BATTLEMODEMENU );
	return true;
}

void CombatMode::CanContinue()
{
	// If player is dead, the game is over
	if ( m_pPlayer->GetCurHP() == 0 )
	{
		AddStringToCombatLog( "You die! Game Over " );
		CGameEngine::Instance()->ContinueGame(m_pCombatant, m_pMonster);		
	}
	// If enemy is dead, game continues
	else if ( m_pCombatant->GetCurHP() == 0 )
	{
		AddStringToCombatLog( "You have slain " + m_pCombatant->GetName() + "!");
		CGameEngine::Instance()->ContinueGame(m_pCombatant, m_pMonster);
	}
	// If it is the enemy his turn, he attacks the player
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





