#pragma warning(disable : 4018)
/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
/****************************************************************
* GameEngine.cpp												*
*																*
* The core of the engine. All calculations are made here.		*
* Think of camera movement, collision detection/response,		*
* clicking on objects and unlocking doors with keys, etcetera.	*		
****************************************************************/
#include "GameEngine.h"
#include "MapLoader.h"

using namespace std;
extern std::string itoa2(const int x);

bool CGameEngine::Load()
{
	// Load own objects
	m_pPrimary->setAmbientLight(ColourValue(0, 0, 0));
	m_pPrimary->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);

	m_pFlashlight = m_pPrimary->createLight("Flashlight");
	m_pFlashlight->setType(Light::LT_SPOTLIGHT);
	m_pFlashlight->setDiffuseColour(1.0, 1.0, 1.0);
	m_pFlashlight->setSpecularColour(1.0, 1.0, 1.0);
	m_pFlashlight->setSpotlightRange(Degree(35), Degree(50));
	m_pFlashlight->setAttenuation( 400, 1.0f, 0.005f, 0.0f );
	m_pFlashlight->setPosition(m_pCamera->getPosition() - (m_pCamera->getDirection() * 50));
	m_pFlashlight->setDirection(m_pCamera->getDirection());

	m_pFlashlight2 = m_pPrimary->createLight("Flashlight2");
	m_pFlashlight2->setType(Light::LT_POINT);
	m_pFlashlight2->setDiffuseColour(1.0, 1.0, 1.0);
	m_pFlashlight2->setSpecularColour(1.0, 1.0, 1.0);
	m_pFlashlight2->setAttenuation( 150, 1.0f, 0.005f, 0.0f );
	m_pFlashlight2->setPosition(m_pCamera->getPosition());

	return true;
}

void CGameEngine::Clean()
{
	// Clean up own objects
	for ( int a = 0; a<m_pDoors.size(); a++ )
	{
		delete m_pDoors[a];
	}
	m_pDoors.clear();

	for ( int a = 0; a<m_pKeys.size(); a++ )
	{
		delete m_pKeys[a];
	}
	m_pKeys.clear();

	for ( int a = 0; a<m_pSwitches.size(); a++ )
	{
		delete m_pSwitches[a];
	}
	m_pSwitches.clear();

	for ( int a = 0; a<m_pLevelItems.size(); a++ )
	{
		delete m_pLevelItems[a];
	}
	m_pLevelItems.clear();

	for ( int a = 0; a<m_pMonsters.size(); a++ )
	{
		delete m_pMonsters[a];
	}
	m_pMonsters.clear();

	m_pPrimary->clearScene();
}

//===============================================================
// FrameListener
//===============================================================
bool CGameEngine::frameStarted(const FrameEvent& evt)
{
	using namespace OIS;

	m_fMessageTime -= evt.timeSinceLastFrame;
	if ( m_fMessageTime < 0 )
	{
		m_pMessageBox->setVisible( false );
	}

	if ( m_iTorchCount == 0 )
	{
		m_fInsanityTimeLeft -= evt.timeSinceLastFrame;
		m_pInsanityBar->setProgress( m_fInsanityTimeLeft / 120.0f );

		if ( m_fInsanityTimeLeft <= 0.0f )
		{
			Clean();
			m_pInsanityBar->setVisible( false );
			m_pMainmenu->setVisible( true );
		}
	}

	if(m_pWindow->isClosed())	return false;

	//Need to capture/update each device
	m_pKeyboard->capture();
	m_pMouse->capture();
	if( m_pJoy ) m_pJoy->capture();

	bool buffJ = (m_pJoy) ? m_pJoy->buffered() : true;

	//Check if one of the devices is not buffered
	if( !m_pMouse->buffered() || !m_pKeyboard->buffered() || !buffJ )
	{
		// one of the input modes is immediate, so setup what is needed for immediate movement
		if (m_fTimeUntilNextToggle >= 0)
			m_fTimeUntilNextToggle -= evt.timeSinceLastFrame;

		// If this is the first frame, pick a speed
		if (evt.timeSinceLastFrame == 0.0f)
		{
			m_fMoveScale = 1;
			m_fRotScale = 0.1;
		}
		// Otherwise scale movement units by time passed since last frame
		else
		{
			// Move about 100 units per second,
			m_fMoveScale = m_fMoveSpeed * evt.timeSinceLastFrame;
			// Take about 10 seconds for full rotation
			m_fRotScale = m_fRotateSpeed * evt.timeSinceLastFrame;
		}

		m_fMoveScale = m_fMoveSpeed * evt.timeSinceLastFrame;
	
		m_vTranslateVector = Ogre::Vector3::ZERO;
	}

	//Check to see which device is not buffered, and handle it
	if( !m_pKeyboard->buffered() )
		if( processUnbufferedKeyInput(evt) == false )
			return false;

	if( !m_pMouse->buffered() || !m_pKeyboard->buffered() || !buffJ )
		moveCamera();

	for ( int a = 0; a<m_pDoors.size(); a++ )
	{
		CDoor *pDoor = m_pDoors[a];
		pDoor->frameStarted( evt );
	}
	for ( int a = 0; a<m_pKeys.size(); a++ )
	{
		CKey *pKey = m_pKeys[a];
		pKey->frameStarted( evt );
	}
	for ( int a = 0; a<m_pSwitches.size(); a++ )
	{
		CSwitch *pSwitch = m_pSwitches[a];
		pSwitch->frameStarted( evt );
	}
	for ( int a = 0; a<m_pLevelItems.size(); a++ )
	{
		CLevelItem *pItem = m_pLevelItems[a];
		pItem->frameStarted( evt );
	}
	for ( int a = 0; a<m_pMonsters.size(); a++ )
	{
		CMonster *pMonster = m_pMonsters[a];
		pMonster->frameStarted( evt );
	}

	if ( m_iTorchCount > 0 )
	{
		m_pTorchescounter->setText( "Torches: " + itoa2(m_iTorchCount) );
		m_pCounterbox->setVisible(true);
	}
	else
		m_pCounterbox->setVisible(false);

	// If the key is picked up.
	if ( m_bKeySelected )
	{
		int clippedKeyX , clippedKeyY , clippedDoorX , clippedDoorY;
		CDoor * door = m_pDoors[0];
		CKey * key = m_pKeys[0];
		// Get position key
		clippedKeyX = (key->getSceneNode()->getPosition().x + 50 - (((int)key->getSceneNode()->getPosition().x + 50) % 100)) / 100;
		clippedKeyY = (key->getSceneNode()->getPosition().z + 50 - (((int)key->getSceneNode()->getPosition().z + 50) % 100)) / 100;

		// Get position door
		clippedDoorX = (door->getSceneNode()->getPosition().x + 50 - (((int)door->getSceneNode()->getPosition().x + 50) % 100)) / 100;
		clippedDoorY = (door->getSceneNode()->getPosition().z + 50 - (((int)door->getSceneNode()->getPosition().z + 50) % 100)) / 100;
		
		// If the key is at the same position as the door, open the door and destroy the key.
		if ( clippedKeyX == clippedDoorX && clippedKeyY == clippedDoorY )
		{
			m_bKeySelected = false;
			door->Unlock();
//			m_pKeys[0]->getSceneNode()->detachAllObjects();									// Detach all object of the node.
//			m_pPrimary->getRootSceneNode()->removeAndDestroyChild( "Switch_Node" );	// Remove and destroy the node from the root.
			m_pPrimary->destroyEntity( "Key" );
			m_pPrimary->getRootSceneNode()->removeAndDestroyChild( "Key_Node" );
			delete m_pKeys[0];																// Remove from vector.
			m_pKeys.clear();

			if ( m_bInAdventureMode )
			{
				m_pMessageBox->setVisible( true );
				m_pMessageBoxText->setText("You have defeated the ninja guard! Onwards to the next dungeon!\nPress ENTER to continue!");
				m_fMessageTime = 5;
				m_bLevelComplete = true;
			}
			else
			{
				m_pAdventure->enable();
				m_pMessageBox->setVisible( true );
				m_pMessageBoxText->setText("You have unlocked adventure mode!");
				m_fMessageTime = 5;
			}
		}
	}

	return true;
}

//===============================================================
// Input
//===============================================================
bool CGameEngine::processUnbufferedKeyInput(const FrameEvent& evt)
{
	using namespace OIS;

	if(m_pKeyboard->isKeyDown(KC_A))
		m_vTranslateVector.x = -m_fMoveScale;	// Move camera left

	if(m_pKeyboard->isKeyDown(KC_D))
		m_vTranslateVector.x = m_fMoveScale;	// Move camera RIGHT

	if(m_pKeyboard->isKeyDown(KC_UP) || m_pKeyboard->isKeyDown(KC_W) )
		m_vTranslateVector.z = -m_fMoveScale;	// Move camera forward

	if(m_pKeyboard->isKeyDown(KC_DOWN) || m_pKeyboard->isKeyDown(KC_S) )
		m_vTranslateVector.z = m_fMoveScale;	// Move camera backward

	if(m_pKeyboard->isKeyDown(KC_RIGHT))
		m_pCamera->yaw(-m_fRotScale * 3);

	if(m_pKeyboard->isKeyDown(KC_LEFT))
		m_pCamera->yaw(m_fRotScale * 3);

	if(m_pKeyboard->isKeyDown(KC_RETURN) && m_bLevelComplete )
	{
		Clean();
		m_pInsanityBar->setVisible( false );
		ShowCity();
	}

	if( m_pKeyboard->isKeyDown(KC_ESCAPE) && m_fTimeUntilNextToggle <= 0 )
	{
		if ( m_pCity->isVisible() )
		{
			SetGUIMode( MAINMENU );
			m_bHadCityOpen = true;
		}
		else if ( m_pMainmenu->isVisible() )
		{
			if ( m_bLoaded )
			{
				if ( m_bInAdventureMode )
					SetGUIMode( ADVENTURE );
				else
					SetGUIMode( NONE );
			}
			else
			{
				if ( m_bHadCityOpen == true )
				{
					SetGUIMode( CITY );
				}
				else
					return false;
			}
		}
		else
		{
			if ( m_bInAdventureMode )
			{
				Clean();
				ShowCity();
			}
			else
			{
				SetGUIMode( MAINMENU );
			}
			m_pMouse->setBuffered(true);
			CEGUI::MouseCursor::getSingleton().setVisible(true);
		}
		m_fTimeUntilNextToggle = 0.3;
	}

	// Calculate torches and show the right message.
	if( m_pKeyboard->isKeyDown(KC_E) && m_fTimeUntilNextToggle <= 0 && m_bInAdventureMode )
	{
		if ( m_iTorchCount > 0 )
		{
			m_iTorchCount -= 1;

			Light *pTorch = m_pPrimary->createLight("Torch" + itoa2(++m_iPlacedTorches));
			pTorch->setType(Light::LT_POINT);
			pTorch->setDiffuseColour(1.0, 0.3, 0.3);
			pTorch->setSpecularColour(1.0, 0.3, 0.3);
			pTorch->setAttenuation( 150, 1.0f, 0.005f, 0.0f );
			pTorch->setPosition(m_pCamera->getPosition());

			if ( m_iTorchCount == 0 )
			{
				m_pMessageBox->setVisible( true );
				m_pMessageBoxText->setText("You ran out of torches! You are losing your mind!");
				m_fMessageTime = 5;
			}
		}
		else
		{
			m_pMessageBox->setVisible( true );
			m_pMessageBoxText->setText("You ran out of torches! You are losing your mind!");
			m_fMessageTime = 5;
		}
		m_fTimeUntilNextToggle = 1;
	}

	if( m_pKeyboard->isKeyDown(KC_F) && m_fTimeUntilNextToggle <= 0 )
	{
		m_bStatsOn = !m_bStatsOn;
		showDebugOverlay(m_bStatsOn);
		m_fTimeUntilNextToggle = 1;
	}

	if( m_pKeyboard->isKeyDown(KC_V) && m_fTimeUntilNextToggle <= 0 )
	{
		// Swap scenemanagers  
		Swap(m_pPrimary, m_pSecondary);
	
		SetupViewport(m_pWindow, m_pPrimary);
	}

	if( m_pKeyboard->isKeyDown(KC_T) && m_fTimeUntilNextToggle <= 0 )
	{
		switch(m_eFiltering)
		{
		case TFO_BILINEAR:
			m_eFiltering = TFO_TRILINEAR;
			m_iAniso = 1;
			break;
		case TFO_TRILINEAR:
			m_eFiltering = TFO_ANISOTROPIC;
			m_iAniso = 8;
			break;
		case TFO_ANISOTROPIC:
			m_eFiltering = TFO_BILINEAR;
			m_iAniso = 1;
			break;
		default: break;
		}
		MaterialManager::getSingleton().setDefaultTextureFiltering(m_eFiltering);
		MaterialManager::getSingleton().setDefaultAnisotropy(m_iAniso);

		showDebugOverlay(m_bStatsOn);
		m_fTimeUntilNextToggle = 1;
	}

	if(m_pKeyboard->isKeyDown(KC_SYSRQ) && m_fTimeUntilNextToggle <= 0)
	{
		std::ostringstream ss;
		ss << "screenshot_" << ++m_iNumScreenShots << ".png";
		m_pWindow->writeContentsToFile(ss.str());
		m_fTimeUntilNextToggle = 0.5;
		m_szDebugText = "Saved: " + ss.str();
	}

	static bool displayCameraDetails = false;
	if(m_pKeyboard->isKeyDown(KC_P) && m_fTimeUntilNextToggle <= 0)
	{
		displayCameraDetails = !displayCameraDetails;
		m_fTimeUntilNextToggle = 0.5;
		if (!displayCameraDetails)
			m_szDebugText = "";
	}

	if(m_pKeyboard->isKeyDown(KC_Q) && m_fTimeUntilNextToggle <=0)
	{
		toggleLights();
		m_fTimeUntilNextToggle = 0.5;
		m_pMessageBox->setVisible( true );
		m_pMessageBoxText->setText("CHEATERR!!!");
		m_fMessageTime = 10;
	}

	// Print camera details
	if(displayCameraDetails)
		m_szDebugText = "P: " + StringConverter::toString(m_pCamera->getDerivedPosition());// +
					 //" " + "O: " + StringConverter::toString(m_pCamera->getDerivedOrientation());

	// Return true to continue rendering
	return true;
}

//===============================================================
// Other
//===============================================================
void CGameEngine::moveCamera()
{
	// Make all the changes to the camera	
	Vector3 pos = m_pCamera->getPosition();
	Vector3 move = m_vTranslateVector;
	move.normalise();
	m_pCamera->moveRelative(move * 10);
	
	// If the player has selected the key
	if ( m_bKeySelected && m_pKeys.size() > 0 )
	{
		// Get the key-scenenode
		SceneNode * scen = m_pKeys[0]->getSceneNode();
		// Set the key in front of the camera.
		scen->setPosition( m_pCamera->getPosition() + m_pCamera->getDirection() * 50 );
		
		// If the camera collides or the key collides, respond.
		if ( m_pMapLoader->hasCube(m_pCamera->getPosition()) || m_pMapLoader->hasCube(scen->getPosition(),true) )
		{
			// Set camera position and don't translate
			m_pCamera->setPosition(pos);
			// Set the key in front of the camera.
			scen->setPosition( m_pCamera->getPosition() + m_pCamera->getDirection() * 50 );
		}
		else // otherwise translate camera and set the key in front of the camera.
		{
			m_pCamera->setPosition(pos);
			m_pCamera->moveRelative(m_vTranslateVector);
			scen->setPosition( m_pCamera->getPosition() + m_pCamera->getDirection() * 50 );
		}
	}
	else // If the key is not selected, only calculate camera.
	{
		if ( m_pMapLoader->hasCube(m_pCamera->getPosition()) )
		{
			m_pCamera->setPosition(pos);
		}
		else
		{
			m_pCamera->setPosition(pos);
			m_pCamera->moveRelative(m_vTranslateVector);
		}
	}

	// Check if we are on top of an item and if we can pick it up
	int clippedX, clippedY; // Clip camera position
	Vector3 position = m_pCamera->getPosition();
	clippedX = (((position.x + 50 - (((int)position.x + 50) % 100)) / 100) - 1) / 3;
	clippedY = (((position.z + 50 - (((int)position.z + 50) % 100)) / 100) - 1) / 3;

	for ( int i = 0; i<m_pLevelItems.size(); i++ )
	{
		CLevelItem *pItem = m_pLevelItems[i];
		if ( pItem->getTileX() == clippedX && pItem->getTileY() == clippedY )
		{
			CItem *pActualItem = m_pFactory->GetRandomItem(rand()%10, m_iLevel);
			if ( m_pPlayer->AddItemToInventory( pActualItem ) )
			{
				string text = "You have picked up an item!\nName:" + pActualItem->GetName() + "\nBonus: +" + itoa2(pActualItem->GetBonus()) + " " + pActualItem->GetBonusString() + "\nValue: " + itoa2(pActualItem->GetValue()) + " goldpieces";
				m_pMessageBox->setVisible( true );
				m_pMessageBoxText->setText(text.c_str());
				m_fMessageTime = 5;

				// Remove item from level
				m_pPrimary->destroyEntity( pItem->getEntity() );
				m_pPrimary->getRootSceneNode()->removeChild(pItem->getNode());
				delete m_pLevelItems[i];
				m_pLevelItems.erase( m_pLevelItems.begin() + i );
				break;
			}
			else
			{
				m_pMessageBox->setVisible( true );
				m_pMessageBoxText->setText("Your inventory is too full! Too bad!\n");
				m_fMessageTime = 3;
				delete pActualItem;
				break;
			}
		}
	}

	// Loop through monsters to check if we need to start combat
	for ( int i = 0; i<m_pMonsters.size(); i++ )
	{
		CMonster *pMonster = m_pMonsters[i];
		if ( pMonster->getTileX() == clippedX && pMonster->getTileY() == clippedY )
		{
			CCombatant *pActualMonster = m_pFactory->GetRandomMonster(m_iLevel);
			// Start combat!
			// Player: m_pPlayer
			// Combatant: pActualMonster
		}
	}

	// Calculate flashlight position and direction, based on the camera position.
	m_pFlashlight->setPosition(m_pCamera->getPosition() - (m_pCamera->getDirection() * 50));
	m_pFlashlight->setDirection(m_pCamera->getDirection());
	m_pFlashlight2->setPosition(m_pCamera->getPosition());

	// Show the light swith tip
	if ( !m_bDisplayedSwitchTip && m_pSwitches.size() > 0 )
	{
		Real dst = m_pSwitches[0]->getSceneNode()->getPosition().distance( m_pCamera->getPosition() );
		if ( dst < 200 )
		{
			m_pMessageBox->setVisible( true );
			m_pMessageBoxText->setText("The floating robot is actually a secret link up to the lighting of the maze!\nClicking on it will give you a hint to the key!");
			m_fMessageTime = 5;
			m_bDisplayedSwitchTip = true;
		}
	}
}

void CGameEngine::toggleLights()
{
	for ( int i = 0; i<keylights.size(); i++ )
	{
		string lightname = keylights[i];
		Light *pLight = m_pPrimary->getLight(lightname);
		pLight->setVisible( !pLight->isVisible() );
	}
}

// GUI
CEGUI::MouseButton CGameEngine::convertButton(OIS::MouseButtonID buttonID)
{
	switch (buttonID)
	{
	case OIS::MB_Left:
		return CEGUI::LeftButton;

	case OIS::MB_Right:
		return CEGUI::RightButton;

	case OIS::MB_Middle:
		return CEGUI::MiddleButton;

	default:
		return CEGUI::LeftButton;
	}
}

bool CGameEngine::mouseMoved(const OIS::MouseEvent &arg)
{
	// If the right mouse button is clicked, rotate the camera. 
	if (arg.state.buttonDown(OIS::MB_Right))
    {
		m_pCamera->yaw(Degree(-0.13 * arg.state.X.rel));
		// If the key is picked up, make sure that the key cannot get in the wall.
		if ( m_bKeySelected && m_pKeys.size() > 0 )
		{
			m_pKeys[0]->getSceneNode()->setPosition( m_pCamera->getPosition() + m_pCamera->getDirection() * 50 );
			if ( m_pMapLoader->hasCube(m_pKeys[0]->getSceneNode()->getPosition()) )
				m_pCamera->yaw(Degree(0.13 * arg.state.X.rel));
			m_pKeys[0]->getSceneNode()->setPosition( m_pCamera->getPosition() + m_pCamera->getDirection() * 50 );
		}
	}

	m_pGUISystem->injectMouseMove(arg.state.X.rel, arg.state.Y.rel);

    return true;
}

bool CGameEngine::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	m_pGUISystem->injectMouseButtonDown(convertButton(id));

	if ( m_pMainmenu->isVisible() == false && m_pCity->isVisible() == false )
	{
		if (id == OIS::MB_Left)
		{
			mLMouseDown = true;
			onLeftPressed(arg);
		}
		if (id == OIS::MB_Right)
		{		
			mRMouseDown = true;
			onRightPressed(arg);
			// Hide the mouse cursor when rotating the camera.
			CEGUI::MouseCursor::getSingleton().hide();		
		}
	}

    return true;
}

bool CGameEngine::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	m_pGUISystem->injectMouseButtonUp(convertButton(id));
	if (id == OIS::MB_Left)
    {
		mLMouseDown = false;
		// If the player was holding the key, the key gets dropped when releasing the mousebutton.
		if ( m_bKeySelected )  
		{
			m_bKeySelected = false;
			m_pKeys[0]->Drop();
			m_pKeys[0]->getSceneNode()->scale( Vector3( 2.5, 2.5, 2.5 ) );

			m_pMessageBox->setVisible( true );
			m_pMessageBoxText->setText("You dropped the ogre head! You can no longer defeat the ninja guard!");
			m_fMessageTime = 5;
		}
	}
	if (id == OIS::MB_Right)
    {
		CEGUI::MouseCursor::getSingleton().show();
		mRMouseDown = false;
	}

    return true;
}

void CGameEngine::onLeftPressed(const OIS::MouseEvent &arg)
{
	// Setup the ray scene query
	CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
	Ray mouseRay = m_pCamera->getCameraToViewportRay(mousePos.d_x/float(arg.state.width), mousePos.d_y/float(arg.state.height));
	m_pRaySceneQuery->setRay(mouseRay);
	m_pRaySceneQuery->setSortByDistance(true);

	// Execute query
	RaySceneQueryResult &result = m_pRaySceneQuery->execute();
	RaySceneQueryResult::iterator itr;

	// Get results, create a node/entity on the position
    for ( itr = result.begin(); itr != result.end(); itr++ )
    {
		if ( !m_bKeySelected )
		{
			// If the object, you clicked on, is a key
			if (itr->movable && itr->movable->getName().substr(0, 3) == "Key")
			{
				if ( m_pKeys.size() > 0 )
				{
					// And the player is in range
					Real dst = m_pKeys[0]->getSceneNode()->getPosition().distance( m_pCamera->getPosition() );
					if ( dst < 150 )
					{
						if ( m_pKeys[0]->getSceneNode() == itr->movable->getParentSceneNode() )
						{			
							// Pick up the key and show the message.
							m_bKeySelected = true;
							m_pKeys[0]->Pickup();
							m_pKeys[0]->getSceneNode()->scale( Vector3( 0.4, 0.4, 0.4 ) );

							m_pMessageBox->setVisible( true );
							m_pMessageBoxText->setText("You picked up the ogre head! You can now defeat the ninja guard!\nRemember, you still rotate with the right mouse button!");
							m_fMessageTime = 5;

							break;
						}
					}
				}
			}
		}
		// If the object, you clicked on, is a lightswitch
		if (itr->movable && itr->movable->getName() == "Switch")
		{
			// Toggle the lights.
			toggleLights();
		}
	}
}

void CGameEngine::onRightPressed(const OIS::MouseEvent &arg)
{
	// Setup the ray scene query
	CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
	Ray mouseRay = m_pCamera->getCameraToViewportRay(mousePos.d_x/float(arg.state.width), mousePos.d_y/float(arg.state.height));
	m_pRaySceneQuery->setRay(mouseRay);
	m_pRaySceneQuery->setSortByDistance(true);

	// Execute query
	RaySceneQueryResult &result = m_pRaySceneQuery->execute();
	RaySceneQueryResult::iterator itr;

	// Get results, create a node/entity on the position
    for ( itr = result.begin(); itr != result.end(); itr++ )
    {
		// If the object, you clicked on, is a key
		if (itr->movable && itr->movable->getName().substr(0, 3) == "Key")
		{
			if ( m_pKeys.size() > 0 )
			{
				if ( m_pKeys[0]->getSceneNode() == itr->movable->getParentSceneNode() )
				{
					// If you already had picked up the key, 
					if ( m_bKeySelected && !mLMouseDown ) // But ignore this if the left mouse button is held down
					{
						// Drop the key and show the message.
						m_bKeySelected = false;
						m_pKeys[0]->Pickup();
						m_pKeys[0]->getSceneNode()->scale( Vector3( 2.5, 2.5, 2.5 ) );

						m_pMessageBox->setVisible( true );
						m_pMessageBoxText->setText("You dropped the ogre head! You can no longer defeat the ninja guard!");
						m_fMessageTime = 5;
					}
					else if ( !mLMouseDown ) // But ignore this if the left mouse button is held down
					{
						// And the player is in range
						Real dst = m_pKeys[0]->getSceneNode()->getPosition().distance( m_pCamera->getPosition() );
						if ( dst < 150 )
						{
							// Pick up the key and show the message.
							m_bKeySelected = true;
							m_pKeys[0]->Drop();
							m_pKeys[0]->getSceneNode()->scale( Vector3( 0.4, 0.4, 0.4 ) );

							m_pMessageBox->setVisible( true );
							m_pMessageBoxText->setText("You picked up the ogre head! You can now defeat the ninja guard!");
							m_fMessageTime = 5;
						}
					}
					break;
				}
			}
		} 
	}
}

bool CGameEngine::LoadLevel( const CEGUI::EventArgs &e )
{
	Clean();
	Load();

	m_iTorchCount = 0;

	SetGUIMode( NONE );
	CEGUI::MouseCursor::getSingleton().setVisible(true);

	keylights = m_pMapLoader->LoadMap( "map.txt", m_pPrimary, m_pCamera );

	m_pFlashlight->setPosition(m_pCamera->getPosition() - (m_pCamera->getDirection() * 50));
	m_pFlashlight->setDirection(m_pCamera->getDirection());
	m_pFlashlight2->setPosition(m_pCamera->getPosition());

	m_bLoaded = true;
	m_bInAdventureMode = false;
	m_bDisplayedSwitchTip = false;

	m_pMessageBox->setVisible( true );
	m_pMessageBoxText->setText("You can rotate the camera by holding down the right mouse button!");
	m_fMessageTime = 5;

	return true;
}

bool CGameEngine::AdventureMode( const CEGUI::EventArgs &e )
{
	m_iLevel = 0;
	m_iTorchCount = 10;

	const CEGUI::WindowEventArgs& we = static_cast<const CEGUI::WindowEventArgs&>(e);
	CEGUI::String senderID = we.window->getName();

	Ogre::LogManager::getSingleton().logMessage("Adventure mode started by: " + string(senderID.c_str()) );
	if ( senderID.find("Easy") != CEGUI::String::npos )
		m_iLevel = 1;
	if ( senderID.find("Medium") != CEGUI::String::npos )
		m_iLevel = 2;
	if ( senderID.find("Hard") != CEGUI::String::npos )
		m_iLevel = 3;
	if ( senderID.find("Challenging") != CEGUI::String::npos )
		m_iLevel = 5;
	if ( senderID.find("Impossible") != CEGUI::String::npos )
		m_iLevel = 7;

	if ( m_iLevel != 0 )
	{
		NextLevel(e);

		m_bInAdventureMode = true;
		m_bDisplayedSwitchTip = false;
		SetGUIMode( ADVENTURE );

		m_pMessageBox->setVisible( true );
		m_pMessageBoxText->setText("In adventure mode you have to defeat the ninja before going insane.\nWhen you run out of torches you will slowly lose your mind in the dark!");
		m_fMessageTime = 10;
	}
	else
	{
		Clean();
		Load();
		m_pMessageBox->setVisible( false );
		m_pMainmenu->setVisible( false );
		ShowCity();
	}

	return true;
}

bool CGameEngine::Quit( const CEGUI::EventArgs &e )
{
	m_bQuit = true;
	return true;
}

bool CGameEngine::NextLevel( const CEGUI::EventArgs &e )
{
	Clean();
	Load();

	m_fInsanityTimeLeft = 120;
	m_pInsanityBar->setVisible( true );
	m_pInsanityBar->setProgress( 1.0f );

	Ogre::LogManager::getSingleton().logMessage("Loading level!");
	SetGUIMode( ADVENTURE );
	CEGUI::MouseCursor::getSingleton().setVisible(true);

	keylights = m_pMapLoader->LoadMap( itoa2(m_iLevel + 5), m_pPrimary, m_pCamera, true );

	m_pFlashlight->setPosition(m_pCamera->getPosition() - (m_pCamera->getDirection() * 50));
	m_pFlashlight->setDirection(m_pCamera->getDirection());
	m_pFlashlight2->setPosition(m_pCamera->getPosition());

	m_bLoaded = true;
	m_bLevelComplete = false;

	m_iTorchCount += m_iLevel;
	m_iLevel += 1;
	return true;
}

void CGameEngine::ShowCity(bool refreshShop)
{
	CEGUI::WindowManager *win = CEGUI::WindowManager::getSingletonPtr();
	vector<CItem *> equipment = m_pPlayer->GetEquipment();
	vector<CItem *> inventory = m_pPlayer->GetInventory();
	m_pCharacterWindow->setText("Hitpoints: " + itoa2(m_pPlayer->GetMaxHP()) + 
								"\nAttack power: " + itoa2(m_pPlayer->GetATP()) + 
								"\nDefensive power: " + itoa2(m_pPlayer->GetDef()) + 
								"\nGold: " + itoa2(m_pPlayer->GetGold()) + " goldpieces");

	SetGUIMode( MAINMENU );

	m_iSelectedType = m_iSelectedIndex = 0;
	m_pItemLabel->setVisible( false );
	m_pItemWindow->setVisible( false );
	m_pBuySellSelected->setVisible( false );
	m_pEquipDequipSelected->setVisible( false );

	// Reset old information
	if ( m_pInventory )
	{
		m_pCity->removeChildWindow( m_pInventory );
		win->destroyWindow( m_pInventory );
		m_pInventory = NULL;
	}
	if ( m_pEquipment )
	{
		m_pCity->removeChildWindow( m_pEquipment );
		win->destroyWindow( m_pEquipment );
		m_pEquipment = NULL;
	}
	if ( m_pItems )
	{
		m_pCity->removeChildWindow( m_pItems );
		win->destroyWindow( m_pItems );
		m_pItems = NULL;
	}
	if ( refreshShop )
	{
		if ( m_pCityItems.size() > 0 )
		{
			for ( int a = 0; a<m_pCityItems.size(); a++ )
				delete m_pCityItems[a];
			m_pCityItems.clear();
		}

		for ( int a = 0; a<15; a++ )
		{
			CItem *pItem = m_pFactory->GetRandomItem(rand()%10, rand()%10);
			m_pCityItems.push_back( pItem );
		}
	}

	// Create inventory, equipment and items panels
	m_pEquipment = win->createWindow("TaharezLook/Listbox", "Root/Equipped");
	m_pEquipment->setSize( CEGUI::UVector2(CEGUI::UDim(0.30, 0), CEGUI::UDim(0.45, 0)) );
	m_pEquipment->setPosition(CEGUI::UVector2(CEGUI::UDim(0.01, 0), CEGUI::UDim(0.05, 0)));
	m_pEquipment->setAlpha(25);
	m_pCity->addChildWindow(m_pEquipment);

	m_pInventory = win->createWindow("TaharezLook/Listbox", "Root/Inventory");
	m_pInventory->setSize( CEGUI::UVector2(CEGUI::UDim(0.30, 0), CEGUI::UDim(0.45, 0)) );
	m_pInventory->setPosition(CEGUI::UVector2(CEGUI::UDim(0.01, 0), CEGUI::UDim(0.54, 0)));
	m_pInventory->setAlpha(25);
	m_pCity->addChildWindow(m_pInventory);

	m_pItems = win->createWindow("TaharezLook/Listbox", "Root/City/Items");
	m_pItems->setSize( CEGUI::UVector2(CEGUI::UDim(0.30, 0), CEGUI::UDim(0.94, 0)) );
	m_pItems->setPosition(CEGUI::UVector2(CEGUI::UDim(0.69, 0), CEGUI::UDim(0.05, 0)));
	m_pItems->setAlpha(25);
	m_pCity->addChildWindow(m_pItems);

	// Populate bars with items
	for ( int a = 0; a<equipment.size(); a++ )
	{
		CItem *pItem = equipment[a];

		string buttonName = "Root/City/Equipment/Button" + itoa2(a);
		CEGUI::Window *m_pItemButton = win->createWindow("TaharezLook/Button", buttonName);
		m_pItemButton->setSize( CEGUI::UVector2(CEGUI::UDim(0.9, 0), CEGUI::UDim(0.09, 0)) );
		m_pItemButton->setPosition( CEGUI::UVector2(CEGUI::UDim(0.05,0), CEGUI::UDim(0.01 + (a * 0.10),0)) );
		m_pItemButton->setText(pItem->GetName());
		m_pItemButton->setAlpha(64);
		m_pItemButton->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::ClickItem, this));
		m_pEquipment->addChildWindow(m_pItemButton);
	}
	for ( int a = 0; a<inventory.size(); a++ )
	{
		CItem *pItem = inventory[a];

		string buttonName = "Root/City/Inventory/Button" + itoa2(a);
		CEGUI::Window *m_pItemButton = win->createWindow("TaharezLook/Button", buttonName);
		m_pItemButton->setSize( CEGUI::UVector2(CEGUI::UDim(0.9, 0), CEGUI::UDim(0.09, 0)) );
		m_pItemButton->setPosition( CEGUI::UVector2(CEGUI::UDim(0.05,0), CEGUI::UDim(0.01 + (a * 0.10),0)) );
		m_pItemButton->setText(pItem->GetName());
		m_pItemButton->setAlpha(64);
		m_pItemButton->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::ClickItem, this));
		m_pInventory->addChildWindow(m_pItemButton);
	}
	for ( int a = 0; a<m_pCityItems.size(); a++ )
	{
		CItem *pItem = NULL;
		pItem = m_pCityItems[a];

		string buttonName = "Root/City/Items/Button" + itoa2(a);
		CEGUI::Window *m_pItemButton = win->createWindow("TaharezLook/Button", buttonName);
		m_pItemButton->setSize( CEGUI::UVector2(CEGUI::UDim(0.9, 0), CEGUI::UDim(0.04, 0)) );
		m_pItemButton->setPosition( CEGUI::UVector2(CEGUI::UDim(0.05,0), CEGUI::UDim(0.01 + (a * 0.05),0)) );
		m_pItemButton->setText(pItem->GetName());
		m_pItemButton->setAlpha(64);
		m_pItemButton->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::ClickItem, this));
		m_pItems->addChildWindow(m_pItemButton);
	}
}

bool CGameEngine::ClickItem( const CEGUI::EventArgs &e )
{
	const CEGUI::WindowEventArgs& we = static_cast<const CEGUI::WindowEventArgs&>(e);
	CEGUI::String senderID = we.window->getName();

	int type = 0;
	if ( senderID.find("Equipment") != CEGUI::String::npos )
		type = 1;
	else if ( senderID.find("Inventory") != CEGUI::String::npos )
		type = 2;
	else if ( senderID.find("Items") != CEGUI::String::npos )
		type = 3;
	if ( type == 0 )
		return true;

	int pos = 10 + ((type == 3) ? 5 : 9) + 7;
	CEGUI::String substr = senderID.substr(pos, senderID.length() - pos);

	string strIndex = substr.c_str();
	std::istringstream strin(strIndex);
	int index;
	strin >> index;
	
	if ( m_iSelectedType == type && m_iSelectedIndex == index )
	{
		m_pItemLabel->setVisible( false );
		m_pItemWindow->setVisible( false );
		m_pBuySellSelected->setVisible( false );
		m_pEquipDequipSelected->setVisible( false );
		m_iSelectedType = m_iSelectedIndex = 0;
	}
	else // Show item
	{
		m_pItemLabel->setVisible( true );
		m_pItemWindow->setVisible( true );
		m_iSelectedType = type;
		m_iSelectedIndex = index;

		CItem *pItem = NULL;
		vector<CItem *> container;

		switch ( m_iSelectedType )
		{
		case 1:
			container = m_pPlayer->GetEquipment();
			m_pBuySellSelected->setText( "Sell item" );
			m_pEquipDequipSelected->setText( "Dequip item" );
			m_pBuySellSelected->setVisible( true );
			m_pEquipDequipSelected->setVisible( true );
			break;
		case 2:
			container = m_pPlayer->GetInventory();
			m_pBuySellSelected->setText( "Sell item" );
			m_pEquipDequipSelected->setText( "Equip item" );
			m_pBuySellSelected->setVisible( true );
			m_pEquipDequipSelected->setVisible( true );
			break;
		case 3:
			container = m_pCityItems;
			m_pBuySellSelected->setText( "Buy item" );
			m_pBuySellSelected->setVisible( true );
			m_pEquipDequipSelected->setVisible( false );
			break;
		default:
			m_pItemLabel->setVisible( false );
			m_pItemWindow->setVisible( false );
			m_pBuySellSelected->setVisible( false );
			m_pEquipDequipSelected->setVisible( false );
			m_iSelectedType = m_iSelectedIndex = 0;
			break;
		}

		if ( container.size() > 0 )
		{
			pItem = container[m_iSelectedIndex];
		}

		if ( pItem != NULL )
		{
			m_pItemWindow->setText( pItem->GetName() + "\n+" +
									itoa2(pItem->GetBonus()) + " " + pItem->GetBonusString() + "\n" +
									"Value: " + itoa2(pItem->GetValue()) + " goldpieces" );
		}
	}

	return true;
}

bool CGameEngine::BuySellItem( const CEGUI::EventArgs &e )
{
	if ( m_iSelectedType != 3 )
	{
		// Sell
		if ( m_iSelectedType == 1 )
			m_pPlayer->SellItemFromEquipment(m_iSelectedIndex);
		else
			m_pPlayer->SellItemFromInventory(m_iSelectedIndex);
		ShowCity(false);
	}
	else
	{
		// Buy
		vector<CItem *> inventory = m_pPlayer->GetInventory();
		CItem *pItem = m_pCityItems[m_iSelectedIndex];
		if ( inventory.size() < 10 && m_pPlayer->GetGold() > pItem->GetValue() )
		{
			m_pCityItems.erase(m_pCityItems.begin() + m_iSelectedIndex);

			m_pPlayer->DeductCash( pItem->GetValue() );
			m_pPlayer->AddItemToInventory(pItem);
			ShowCity(false);
		}
	}

	return true;
}

bool CGameEngine::EquipDequipItem( const CEGUI::EventArgs &e )
{
	vector<CItem *> equipment = m_pPlayer->GetEquipment();
	vector<CItem *> inventory = m_pPlayer->GetInventory();
	if ( m_iSelectedType == 1 )
	{
		// Dequip
		if ( inventory.size() < 10 )
		{
			if ( m_pPlayer->DequipItemFromEquipment(m_iSelectedIndex) )
				ShowCity( false );
		}
	}
	else if ( m_iSelectedType == 2 )
	{
		// Equip
		if ( equipment.size() < 10 )
		{
			if ( m_pPlayer->EquipItemFromInventory(m_iSelectedIndex) )
				ShowCity( false );
		}
	}
	return true;
}

void CGameEngine::SetGUIMode( GUIMode mode )
{
	m_pCity->setVisible( false );
	m_pEasyLevel->setVisible( false );
	m_pMediumLevel->setVisible( false );
	m_pHardLevel->setVisible( false );
	m_pChallengingLevel->setVisible( false );
	m_pImpossibleLevel->setVisible( false );
	m_pMainmenu->setVisible( false );
	m_pCounterbox->setVisible( false );
	m_pInsanityBar->setVisible( false );

	switch (mode)
	{
	case MAINMENU:
		m_pMainmenu->setVisible( true );
		break;
	case CITY:
		m_pCity->setVisible( true );
		m_pEasyLevel->setVisible( true );
		m_pMediumLevel->setVisible( true );
		m_pHardLevel->setVisible( true );
		m_pChallengingLevel->setVisible( true );
		m_pImpossibleLevel->setVisible( true );
		break;
	case ADVENTURE:
		m_pCounterbox->setVisible( true );
		m_pInsanityBar->setVisible( false );
		break;
	case NONE:
	default:
		break;
	}
}
