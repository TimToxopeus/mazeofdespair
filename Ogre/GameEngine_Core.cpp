/****************************************************************
* GameEngine_Core.cpp											*
*																*
* Contains all the core code to keep the specific code cleanly  *
* seperate from the core in GameEngine.cpp						*
****************************************************************/
#include "GameEngine.h"

CGameEngine *CGameEngine::m_pInstance = NULL;

//===============================================================
// Core
//===============================================================
CGameEngine::CGameEngine()
{
    m_pRoot = NULL;
    m_pCamera = NULL;
	m_pCameraNode = NULL;

    m_pPrimary = NULL;
    m_pWindow = NULL;
	m_pDebugOverlay = NULL;
	m_pInputManager = NULL;
	m_pMouse = NULL;
	m_pKeyboard = NULL;
	m_pJoy = NULL;

	m_vTranslateVector = Vector3::ZERO;
	m_bStatsOn = true;
	m_iNumScreenShots = 0;
	m_eFiltering = TFO_BILINEAR;
	m_iAniso = 1;
	m_iSceneDetailIndex = 0;

	m_fMoveScale = 0.0f;
	m_fRotScale = 0.0f;
	m_fTimeUntilNextToggle = 0;
	m_fMoveSpeed = 500;
	m_fRotateSpeed = 36;

	m_iTorchCount = 0;
	m_iPlacedTorches = 0;
	m_pMapLoader = NULL;
	m_fInsanityTimeLeft = 120;
	m_bKeySelected = false;
	m_bLevelComplete = false;
	m_bLoaded = false;
	m_bInAdventureMode = false;
	m_bDisplayedSwitchTip = false;
	m_bHadCityOpen = true;
	m_bMapVisible = false;

	m_pGUIRenderer = NULL;
	m_pGUISystem = NULL;

	m_pPlayer = NULL;
	m_pFactory = NULL;

	m_pItems = NULL;
	m_pInventory = NULL;
	m_pEquipment = NULL;
}

bool CGameEngine::Init()
{
	// Initialise all Ogre related objects
	m_pRoot = new Root("plugins.cfg", "ogre.cfg", "ogre.log");

    ConfigFile cf;
    cf.load("resources.cfg");

    // Go through all sections & settings in the file
    ConfigFile::SectionIterator seci = cf.getSectionIterator();

    String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        ConfigFile::SettingsMultiMap *settings = seci.getNext();
        ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }

    if(m_pRoot->showConfigDialog())
        m_pWindow = m_pRoot->initialise(true);
    else
        return false;

	// Create scene manager
	m_pPrimary = m_pRoot->createSceneManager(ST_GENERIC, "ExampleSMInstance");
	m_pSecondary = m_pRoot->createSceneManager(ST_GENERIC, "secondary");

	// Create camera
	m_pCamera = m_pPrimary->createCamera(CAMERA_NAME);
    m_pCamera->setPosition(Vector3(0,0,500));
    m_pCamera->lookAt(Vector3(0,0,-300));
    m_pCamera->setNearClipDistance(5);

	m_pCombatCamera = m_pSecondary->createCamera(CAMERA_NAME);

	SetupViewport(m_pWindow, m_pPrimary);

	// Set default number of mipmaps
	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	// Initialise resources
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	// Assign the frame listener
	m_pRoot->addFrameListener(this);

	using namespace OIS;

	m_pDebugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");

	LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
	ParamList pl;
	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	m_pWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

	m_pInputManager = InputManager::createInputSystem( pl );

	//Create all devices (We only catch joystick exceptions here, as, most people have Key/Mouse)
	m_pKeyboard = static_cast<Keyboard*>(m_pInputManager->createInputObject( OISKeyboard, false ));
	m_pMouse = static_cast<Mouse*>(m_pInputManager->createInputObject( OISMouse, true ));
	try {
		m_pJoy = static_cast<JoyStick*>(m_pInputManager->createInputObject( OISJoyStick, false ));
	}
	catch(...) {
		m_pJoy = 0;
	}

	m_pMouse->setEventCallback(this);

	// Create RaySceneQuery
	m_pRaySceneQuery = m_pPrimary->createRayQuery(Ray());

	//Set initial mouse clipping size
	windowResized(m_pWindow);

	showDebugOverlay(true);

	//Register as a Window listener
	WindowEventUtilities::addWindowEventListener(m_pWindow, this);
	m_bStatsOn = false;
	showDebugOverlay(false);

	// Other stuff
	m_pMapLoader = new CMapLoader();
	m_pPlayer = new CPlayer();
	m_pFactory = new CRandomizedFactory();

	// GUI for the primary scenemanager
	m_pGUIRenderer = new CEGUI::OgreCEGUIRenderer(m_pWindow, Ogre::RENDER_QUEUE_OVERLAY, false, 3000, m_pPrimary);
	m_pGUISystem = new CEGUI::System(m_pGUIRenderer);
	CEGUI::SchemeManager::getSingleton().loadScheme((CEGUI::utf8*)"TaharezLookSkin.scheme");
	m_pGUISystem->setDefaultMouseCursor((CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
	CEGUI::MouseCursor::getSingleton().setImage(CEGUI::System::getSingleton().getDefaultMouseCursor());
	m_pGUISystem->setDefaultFont((CEGUI::utf8*)"BlueHighway-12");

	m_pWindowManager = CEGUI::WindowManager::getSingletonPtr();
	m_pSheet = m_pWindowManager->createWindow("DefaultGUISheet", "Root");

	m_pMainmenu = m_pWindowManager->createWindow("TaharezLook/FrameWindow", "Root/Mainmenu");
	m_pMainmenu->setSize( CEGUI::UVector2(CEGUI::UDim(0.3, 0), CEGUI::UDim(0.3, 0)) );
	m_pMainmenu->setPosition( CEGUI::UVector2(CEGUI::UDim(0.35,0), CEGUI::UDim(0.35,0)) );
	m_pSheet->addChildWindow(m_pMainmenu);

	m_pInsanityBar = (CEGUI::ProgressBar *)m_pWindowManager->createWindow("TaharezLook/ProgressBar", "Root/InsanityBar");
	m_pInsanityBar->setSize( CEGUI::UVector2(CEGUI::UDim(0.6, 0), CEGUI::UDim(0.03, 0)) );
	m_pInsanityBar->setPosition( CEGUI::UVector2(CEGUI::UDim(0.2,0), CEGUI::UDim(0.01,0)) );
	m_pSheet->addChildWindow(m_pInsanityBar);

	m_pMessageBox = m_pWindowManager->createWindow("TaharezLook/FrameWindow", "Root/MessageBox");
	m_pMessageBox->setSize( CEGUI::UVector2(CEGUI::UDim(0.6, 0), CEGUI::UDim(0.1, 0)) );
	m_pMessageBox->setPosition( CEGUI::UVector2(CEGUI::UDim(0.2,0), CEGUI::UDim(0.05,0)) );
	m_pSheet->addChildWindow(m_pMessageBox);

	m_pMessageBoxText = m_pWindowManager->createWindow("TaharezLook/StaticText", "Root/MessageBox/Text" );
	m_pMessageBoxText->setSize( CEGUI::UVector2(CEGUI::UDim(0.9, 0), CEGUI::UDim(0.7, 0)) );
	m_pMessageBoxText->setPosition(CEGUI::UVector2(CEGUI::UDim(0.05, 0), CEGUI::UDim(0.25, 0)));
	m_pMessageBox->addChildWindow(m_pMessageBoxText);

	CEGUI::Window *load = m_pWindowManager->createWindow("TaharezLook/Button", "Root/Mainmenu/LoadButton");
	load->setText("Tutorial");
	load->setSize(CEGUI::UVector2(CEGUI::UDim(0.6, 0), CEGUI::UDim(0.2, 0)));
	load->setPosition(CEGUI::UVector2(CEGUI::UDim(0.2, 0), CEGUI::UDim(0.15, 0)));
	load->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::LoadLevel, this));
	m_pMainmenu->addChildWindow(load);

	m_pAdventure = m_pWindowManager->createWindow("TaharezLook/Button", "Root/Mainmenu/AdventureButton");
	m_pAdventure->setText("Adventure mode");
	m_pAdventure->setSize(CEGUI::UVector2(CEGUI::UDim(0.6, 0), CEGUI::UDim(0.2, 0)));
	m_pAdventure->setPosition(CEGUI::UVector2(CEGUI::UDim(0.2, 0), CEGUI::UDim(0.45, 0)));
	m_pAdventure->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::AdventureMode, this));
	//m_pAdventure->disable();
	m_pMainmenu->addChildWindow(m_pAdventure);

	CEGUI::Window *quit = m_pWindowManager->createWindow("TaharezLook/Button", "Root/Mainmenu/QuitButton");
	quit->setText("Quit game");
	quit->setSize(CEGUI::UVector2(CEGUI::UDim(0.6, 0), CEGUI::UDim(0.2, 0)));
	quit->setPosition(CEGUI::UVector2(CEGUI::UDim(0.2, 0), CEGUI::UDim(0.75, 0)));
	quit->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::Quit, this));
	m_pMainmenu->addChildWindow(quit);

	m_pCounterbox = m_pWindowManager->createWindow("TaharezLook/Listbox", "Root/Counter" );
	m_pCounterbox->setSize( CEGUI::UVector2(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.05, 0)) );
	m_pSheet->addChildWindow(m_pCounterbox);

	m_pTorchescounter = m_pWindowManager->createWindow("TaharezLook/StaticText", "Root/Counter/TorchesCounter" );
	m_pTorchescounter->setSize( CEGUI::UVector2(CEGUI::UDim(0.9, 0), CEGUI::UDim(0.8, 0)) );
	m_pTorchescounter->setPosition(CEGUI::UVector2(CEGUI::UDim(0.05, 0), CEGUI::UDim(0.1, 0)));
	m_pTorchescounter->setText("Torches: 10");
	m_pCounterbox->addChildWindow(m_pTorchescounter);

	m_pCounterbox->setVisible(false);
	m_pMessageBox->setVisible(false);
	m_pInsanityBar->setVisible(false);

	CEGUI::Texture *cTex = m_pGUIRenderer->createTexture("map2.jpg");
	CEGUI::Imageset *imageSet = CEGUI::ImagesetManager::getSingleton().createImageset((CEGUI::utf8*)"MapSet", cTex);
	imageSet->defineImage((CEGUI::utf8*)"map2.jpg", CEGUI::Point(0.0f, 0.0f), CEGUI::Size(cTex->getWidth(), cTex->getHeight()),	CEGUI::Point(0.0f,0.0f));

	m_pMap = m_pWindowManager->createWindow("TaharezLook/StaticImage", "Root/Map");
	m_pMap->setSize( CEGUI::UVector2(CEGUI::UDim(0.9, 0), CEGUI::UDim(0.8, 0)) );
	m_pMap->setPosition( CEGUI::UVector2(CEGUI::UDim(0.05,0), CEGUI::UDim(0.1,0)) );
	m_pMap->setProperty("Image", CEGUI::PropertyHelper::imageToString(&imageSet->getImage((CEGUI::utf8*)"map2.jpg")));
	m_pSheet->addChildWindow(m_pMap);
	m_pMap->setVisible( false );

	cTex = m_pGUIRenderer->createTexture("player.jpg");
	imageSet = CEGUI::ImagesetManager::getSingleton().createImageset((CEGUI::utf8*)"PlayerSet", cTex);
	imageSet->defineImage((CEGUI::utf8*)"player.jpg", CEGUI::Point(0.0f, 0.0f), CEGUI::Size(cTex->getWidth(), cTex->getHeight()),	CEGUI::Point(0.0f,0.0f));

	m_pMapPlayer = m_pWindowManager->createWindow("TaharezLook/StaticImage", "Root/Map/Player");
	m_pMapPlayer->setSize( CEGUI::UVector2(CEGUI::UDim(0.9, 0), CEGUI::UDim(0.8, 0)) );
	m_pMapPlayer->setPosition( CEGUI::UVector2(CEGUI::UDim(0.05,0), CEGUI::UDim(0.1,0)) );
	m_pMapPlayer->setProperty("Image", CEGUI::PropertyHelper::imageToString(&imageSet->getImage((CEGUI::utf8*)"player.jpg")));
	m_pMap->addChildWindow(m_pMapPlayer);

	cTex = m_pGUIRenderer->createTexture("Dirt.jpg");
	m_pMapWallSet = CEGUI::ImagesetManager::getSingleton().createImageset((CEGUI::utf8*)"WallSet", cTex);
	m_pMapWallSet->defineImage((CEGUI::utf8*)"Dirt.jpg", CEGUI::Point(0.0f, 0.0f), CEGUI::Size(cTex->getWidth(), cTex->getHeight()),	CEGUI::Point(0.0f,0.0f));

	//======================================================================================================
	// CITY GUI
	//======================================================================================================
	cTex = m_pGUIRenderer->createTexture("stad.jpg");
	imageSet = CEGUI::ImagesetManager::getSingleton().createImageset((CEGUI::utf8*)"CitySet", cTex);
	imageSet->defineImage((CEGUI::utf8*)"stad.jpg", CEGUI::Point(0.0f, 0.0f), CEGUI::Size(cTex->getWidth(), cTex->getHeight()),	CEGUI::Point(0.0f,0.0f));

	m_pCity = m_pWindowManager->createWindow("TaharezLook/StaticImage", "Root/City");
	m_pCity->setSize( CEGUI::UVector2(CEGUI::UDim(0.9, 0), CEGUI::UDim(0.8, 0)) );
	m_pCity->setPosition( CEGUI::UVector2(CEGUI::UDim(0.05,0), CEGUI::UDim(0.1,0)) );
	m_pCity->setProperty("Image", CEGUI::PropertyHelper::imageToString(&imageSet->getImage((CEGUI::utf8*)"stad.jpg")));
	m_pSheet->addChildWindow(m_pCity);

	m_pEasyLevel = m_pWindowManager->createWindow("TaharezLook/Button", "Root/Adventure/Easy");
	m_pEasyLevel->setText("Easy dungeon");
	m_pEasyLevel->setSize(CEGUI::UVector2(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.07, 0)));
	m_pEasyLevel->setPosition(CEGUI::UVector2(CEGUI::UDim(0.05, 0), CEGUI::UDim(0.91, 0)));
	m_pEasyLevel->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::AdventureMode, this));
	m_pSheet->addChildWindow(m_pEasyLevel);

	m_pMediumLevel = m_pWindowManager->createWindow("TaharezLook/Button", "Root/Adventure/Medium");
	m_pMediumLevel->setText("Medium dungeon");
	m_pMediumLevel->setSize(CEGUI::UVector2(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.07, 0)));
	m_pMediumLevel->setPosition(CEGUI::UVector2(CEGUI::UDim(0.24, 0), CEGUI::UDim(0.91, 0)));
	m_pMediumLevel->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::AdventureMode, this));
	m_pSheet->addChildWindow(m_pMediumLevel);

	m_pHardLevel = m_pWindowManager->createWindow("TaharezLook/Button", "Root/Adventure/Hard");
	m_pHardLevel->setText("Hard dungeon");
	m_pHardLevel->setSize(CEGUI::UVector2(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.07, 0)));
	m_pHardLevel->setPosition(CEGUI::UVector2(CEGUI::UDim(0.43, 0), CEGUI::UDim(0.91, 0)));
	m_pHardLevel->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::AdventureMode, this));
	m_pSheet->addChildWindow(m_pHardLevel);

	m_pChallengingLevel = m_pWindowManager->createWindow("TaharezLook/Button", "Root/Adventure/Challenging");
	m_pChallengingLevel->setText("Crazy dungeon");
	m_pChallengingLevel->setSize(CEGUI::UVector2(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.07, 0)));
	m_pChallengingLevel->setPosition(CEGUI::UVector2(CEGUI::UDim(0.62, 0), CEGUI::UDim(0.91, 0)));
	m_pChallengingLevel->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::AdventureMode, this));
	m_pSheet->addChildWindow(m_pChallengingLevel);

	m_pImpossibleLevel = m_pWindowManager->createWindow("TaharezLook/Button", "Root/Adventure/Impossible");
	m_pImpossibleLevel->setText("Insane dungeon");
	m_pImpossibleLevel->setSize(CEGUI::UVector2(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.07, 0)));
	m_pImpossibleLevel->setPosition(CEGUI::UVector2(CEGUI::UDim(0.80, 0), CEGUI::UDim(0.91, 0)));
	m_pImpossibleLevel->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::AdventureMode, this));
	m_pSheet->addChildWindow(m_pImpossibleLevel);

	CEGUI::Window *m_pEquipmentLabel = m_pWindowManager->createWindow("TaharezLook/StaticText", "Root/City/EquipmentLabel" );
	m_pEquipmentLabel->setSize( CEGUI::UVector2(CEGUI::UDim(0.30, 0), CEGUI::UDim(0.05, 0)) );
	m_pEquipmentLabel->setPosition(CEGUI::UVector2(CEGUI::UDim(0.01, 0), CEGUI::UDim(0.01, 0)));
	m_pEquipmentLabel->setText("Equipment");
	m_pEquipmentLabel->setAlpha(64);
	m_pCity->addChildWindow(m_pEquipmentLabel);

	CEGUI::Window *m_pInventoryLabel = m_pWindowManager->createWindow("TaharezLook/StaticText", "Root/City/InventoryLabel" );
	m_pInventoryLabel->setSize( CEGUI::UVector2(CEGUI::UDim(0.30, 0), CEGUI::UDim(0.05, 0)) );
	m_pInventoryLabel->setPosition(CEGUI::UVector2(CEGUI::UDim(0.01, 0), CEGUI::UDim(0.50, 0)));
	m_pInventoryLabel->setText("Inventory");
	m_pInventoryLabel->setAlpha(64);
	m_pCity->addChildWindow(m_pInventoryLabel);

	CEGUI::Window *m_pShopLabel = m_pWindowManager->createWindow("TaharezLook/StaticText", "Root/City/ItemsLabel" );
	m_pShopLabel->setSize( CEGUI::UVector2(CEGUI::UDim(0.30, 0), CEGUI::UDim(0.05, 0)) );
	m_pShopLabel->setPosition(CEGUI::UVector2(CEGUI::UDim(0.69, 0), CEGUI::UDim(0.01, 0)));
	m_pShopLabel->setText("Ye olde shoppe");
	m_pShopLabel->setAlpha(64);
	m_pCity->addChildWindow(m_pShopLabel);

	CEGUI::Window *m_pStatsLabel = m_pWindowManager->createWindow("TaharezLook/StaticText", "Root/City/StatsLabel" );
	m_pStatsLabel->setSize( CEGUI::UVector2(CEGUI::UDim(0.30, 0), CEGUI::UDim(0.05, 0)) );
	m_pStatsLabel->setPosition(CEGUI::UVector2(CEGUI::UDim(0.35, 0), CEGUI::UDim(0.01, 0)));
	m_pStatsLabel->setText("Your character");
	m_pStatsLabel->setAlpha(64);
	m_pCity->addChildWindow(m_pStatsLabel);

	m_pCharacterWindow = m_pWindowManager->createWindow("TaharezLook/StaticText", "Root/City/CharacterWindow" );
	m_pCharacterWindow->setSize( CEGUI::UVector2(CEGUI::UDim(0.30, 0), CEGUI::UDim(0.20, 0)) );
	m_pCharacterWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.35, 0), CEGUI::UDim(0.05, 0)));
	m_pCharacterWindow->setAlpha(25);
	m_pCity->addChildWindow(m_pCharacterWindow);

	m_pItemLabel = m_pWindowManager->createWindow("TaharezLook/StaticText", "Root/City/ItemLabel" );
	m_pItemLabel->setSize( CEGUI::UVector2(CEGUI::UDim(0.30, 0), CEGUI::UDim(0.05, 0)) );
	m_pItemLabel->setPosition(CEGUI::UVector2(CEGUI::UDim(0.35, 0), CEGUI::UDim(0.50, 0)));
	m_pItemLabel->setText("Selected item");
	m_pItemLabel->setAlpha(64);
	m_pCity->addChildWindow(m_pItemLabel);

	m_pItemWindow = m_pWindowManager->createWindow("TaharezLook/StaticText", "Root/City/ItemWindow" );
	m_pItemWindow->setSize( CEGUI::UVector2(CEGUI::UDim(0.30, 0), CEGUI::UDim(0.20, 0)) );
	m_pItemWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.35, 0), CEGUI::UDim(0.54, 0)));
	m_pItemWindow->setText("THE SWORD OF ABSOLUTE TRUTH\n+5 ATP\nValue: 3500 Gold");
	m_pItemWindow->setAlpha(25);
	m_pCity->addChildWindow(m_pItemWindow);

	m_pBuySellSelected = m_pWindowManager->createWindow("TaharezLook/Button", "Root/City/BuySellButton");
	m_pBuySellSelected->setText("Buy item");
	m_pBuySellSelected->setSize(CEGUI::UVector2(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.04, 0)));
	m_pBuySellSelected->setPosition(CEGUI::UVector2(CEGUI::UDim(0.35, 0), CEGUI::UDim(0.75, 0)));
	m_pBuySellSelected->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::BuySellItem, this));
	m_pCity->addChildWindow(m_pBuySellSelected);

	m_pEquipDequipSelected = m_pWindowManager->createWindow("TaharezLook/Button", "Root/City/EquipDequipButton");
	m_pEquipDequipSelected->setText("Equip item");
	m_pEquipDequipSelected->setSize(CEGUI::UVector2(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.04, 0)));
	m_pEquipDequipSelected->setPosition(CEGUI::UVector2(CEGUI::UDim(0.50, 0), CEGUI::UDim(0.75, 0)));
	m_pEquipDequipSelected->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::EquipDequipItem, this));
	m_pCity->addChildWindow(m_pEquipDequipSelected);

	m_pCity->setVisible( false );
	m_pEasyLevel->setVisible( false );
	m_pMediumLevel->setVisible( false );
	m_pHardLevel->setVisible( false );
	m_pChallengingLevel->setVisible( false );
	m_pImpossibleLevel->setVisible( false );

	// Set gui system
	m_pGUISystem->setGUISheet(m_pSheet);


	// GUI for the secondary scenemanager

	return true;
}

void CGameEngine::SetupViewport(RenderWindow *pWindow, SceneManager *pCurrent)
{
    pWindow->removeAllViewports();

    Camera *cam = pCurrent->getCamera(CAMERA_NAME);
    Viewport *vp = pWindow->addViewport(cam);

    vp->setBackgroundColour(ColourValue(0,0,0));
    cam->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
    vp->setOverlaysEnabled(true);
}

void CGameEngine::Swap(SceneManager *&first, SceneManager *&second)
{
    SceneManager *tmp = first;
    first = second;
    second = tmp;
}

void CGameEngine::Run()
{
	m_bQuit = false;
	m_bLoaded = false;
	m_pRoot->startRendering();
}

void CGameEngine::Shutdown()
{
	// Clean up Ogre related objects
    if (m_pRoot)
	{
        delete m_pRoot;
	}

	if ( m_pMapLoader )
	{
		delete m_pMapLoader;
		m_pMapLoader = NULL;
	}
	if ( m_pPlayer )
	{
		delete m_pPlayer;
		m_pPlayer = NULL;
	}
	if ( m_pFactory )
	{
		delete m_pFactory;
		m_pFactory = NULL;
	}

	WindowEventUtilities::removeWindowEventListener(m_pWindow, this);
	windowClosed(m_pWindow);

    m_pRoot = NULL;
    m_pCamera = NULL;
	m_pCameraNode = NULL;
    m_pPrimary = NULL;
    m_pWindow = NULL;
	m_pDebugOverlay = NULL;
	m_pInputManager = NULL;
	m_pMouse = NULL;
	m_pKeyboard = NULL;
	m_pJoy = NULL;

	m_vTranslateVector = Vector3::ZERO;
	m_bStatsOn = true;
	m_iNumScreenShots = 0;
	m_eFiltering = TFO_BILINEAR;
	m_iAniso = 1;
	m_iSceneDetailIndex = 0;

	m_fMoveScale = 0.0f;
	m_fRotScale = 0.0f;
	m_fTimeUntilNextToggle = 0;
	m_fMoveSpeed = 0;
	m_fRotateSpeed = 36;

	m_iTorchCount = 0;
	m_pMapLoader = NULL;

	m_pGUIRenderer = NULL;
	m_pGUISystem = NULL;
}

//===============================================================
// Other
//===============================================================
void CGameEngine::showDebugOverlay(bool show)
{
	if (m_pDebugOverlay)
	{
		if (show)
			m_pDebugOverlay->show();
		else
			m_pDebugOverlay->hide();
	}
}

void CGameEngine::updateStats()
{
	static String currFps = "Current FPS: ";
	static String avgFps = "Average FPS: ";
	static String bestFps = "Best FPS: ";
	static String worstFps = "Worst FPS: ";
	static String tris = "Triangle Count: ";
	static String batches = "Batch Count: ";

	// update stats when necessary
	try {
		OverlayElement* guiAvg = OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
		OverlayElement* guiCurr = OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
		OverlayElement* guiBest = OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
		OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");

		const RenderTarget::FrameStats& stats = m_pWindow->getStatistics();
		guiAvg->setCaption(avgFps + StringConverter::toString(stats.avgFPS));
		guiCurr->setCaption(currFps + StringConverter::toString(stats.lastFPS));
		guiBest->setCaption(bestFps + StringConverter::toString(stats.bestFPS)
			+" "+StringConverter::toString(stats.bestFrameTime)+" ms");
		guiWorst->setCaption(worstFps + StringConverter::toString(stats.worstFPS)
			+" "+StringConverter::toString(stats.worstFrameTime)+" ms");

		OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
		guiTris->setCaption(tris + StringConverter::toString(stats.triangleCount));

		OverlayElement* guiBatches = OverlayManager::getSingleton().getOverlayElement("Core/NumBatches");
		guiBatches->setCaption(batches + StringConverter::toString(stats.batchCount));

		OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
		guiDbg->setCaption(m_szDebugText);
	}
	catch(...) { /* ignore */ }
}

//===============================================================
// FrameListener
//===============================================================
bool CGameEngine::frameEnded(const FrameEvent& evt)
{
	updateStats();
	if ( m_bQuit )
		return false;
	return true;
}

//===============================================================
// WindowEventListener
//===============================================================
//Adjust mouse clipping area
void CGameEngine::windowResized(RenderWindow* rw)
{
	unsigned int width, height, depth;
	int left, top;
	rw->getMetrics(width, height, depth, left, top);

	const OIS::MouseState &ms = m_pMouse->getMouseState();
	ms.width = width;
	ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void CGameEngine::windowClosed(RenderWindow* rw)
{
	//Only close for window that created OIS (the main window in these demos)
	if( rw == m_pWindow )
	{
		if( m_pInputManager )
		{
			m_pInputManager->destroyInputObject( m_pMouse );
			m_pInputManager->destroyInputObject( m_pKeyboard );
			m_pInputManager->destroyInputObject( m_pJoy );

			OIS::InputManager::destroyInputSystem(m_pInputManager);
			m_pInputManager = 0;
		}
	}
}

