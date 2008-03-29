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
		m_pCamera->yaw(-m_fRotScale);

	if(m_pKeyboard->isKeyDown(KC_RETURN) && m_bLevelComplete )
		NextLevel();

	if(m_pKeyboard->isKeyDown(KC_LEFT))
		m_pCamera->yaw(m_fRotScale);

	if( m_pKeyboard->isKeyDown(KC_ESCAPE) && m_fTimeUntilNextToggle <= 0 )
	{
		if ( m_pMainmenu->isVisible() )
		{
			if ( m_bLoaded )
			{
				m_pMainmenu->setVisible(false);
			}
			else
			{
				return false;
			}
		}
		else
		{
			m_pMainmenu->setVisible(true);
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

	m_pMainmenu->setVisible(false);
	m_pCounterbox->setVisible(false);
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

	// No insanity bar in normal level mode
	m_pInsanityBar->setVisible( false );

	return true;
}

bool CGameEngine::AdventureMode( const CEGUI::EventArgs &e )
{
	m_iLevel = 0;
	m_iTorchCount = 10;
	NextLevel();

	m_bInAdventureMode = true;
	m_bDisplayedSwitchTip = false;

	m_pMessageBox->setVisible( true );
	m_pMessageBoxText->setText("In adventure mode you have to defeat the ninja before going insane.\nWhen you run out of torches you will slowly lose your mind in the dark!");
	m_fMessageTime = 10;

	return true;
}

bool CGameEngine::Quit( const CEGUI::EventArgs &e )
{
	m_bQuit = true;
	return true;
}

void CGameEngine::NextLevel()
{
	Clean();
	Load();

	m_fInsanityTimeLeft = 120;
	m_pInsanityBar->setVisible( true );
	m_pInsanityBar->setProgress( 1.0f );

	Ogre::LogManager::getSingleton().logMessage("Loading level!");
	m_pMainmenu->setVisible(false);
	m_pCounterbox->setVisible(true);
	CEGUI::MouseCursor::getSingleton().setVisible(true);

	keylights = m_pMapLoader->LoadMap( itoa2(m_iLevel + 5), m_pPrimary, m_pCamera, true );

	m_pFlashlight->setPosition(m_pCamera->getPosition() - (m_pCamera->getDirection() * 50));
	m_pFlashlight->setDirection(m_pCamera->getDirection());
	m_pFlashlight2->setPosition(m_pCamera->getPosition());

	m_bLoaded = true;
	m_bLevelComplete = false;

	m_iTorchCount += m_iLevel;
	m_iLevel += 1;
}
