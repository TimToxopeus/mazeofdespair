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

    m_pSceneManager = NULL;
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

	m_pGUIRenderer = NULL;
	m_pGUISystem = NULL;
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
	m_pSceneManager = m_pRoot->createSceneManager(ST_GENERIC, "ExampleSMInstance");
	
	// Create camera
	m_pCamera = m_pSceneManager->createCamera("PlayerCam");
    m_pCamera->setPosition(Vector3(0,0,500));
    m_pCamera->lookAt(Vector3(0,0,-300));
    m_pCamera->setNearClipDistance(5);
	// Set camera on a node? m_pCameraNode

	// Create viewport
    Viewport* vp = m_pWindow->addViewport(m_pCamera);
    vp->setBackgroundColour(ColourValue(0,0,0));
    m_pCamera->setAspectRatio( Real(vp->getActualWidth()) / Real(vp->getActualHeight()) );
	
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
	m_pRaySceneQuery = m_pSceneManager->createRayQuery(Ray());

	//Set initial mouse clipping size
	windowResized(m_pWindow);

	showDebugOverlay(true);

	//Register as a Window listener
	WindowEventUtilities::addWindowEventListener(m_pWindow, this);

	// GUI
	m_pGUIRenderer = new CEGUI::OgreCEGUIRenderer(m_pWindow, Ogre::RENDER_QUEUE_OVERLAY, false, 3000, m_pSceneManager);
	m_pGUISystem = new CEGUI::System(m_pGUIRenderer);
	CEGUI::SchemeManager::getSingleton().loadScheme((CEGUI::utf8*)"TaharezLookSkin.scheme");
	m_pGUISystem->setDefaultMouseCursor((CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
	CEGUI::MouseCursor::getSingleton().setImage(CEGUI::System::getSingleton().getDefaultMouseCursor());
	m_pGUISystem->setDefaultFont((CEGUI::utf8*)"BlueHighway-12");

	CEGUI::WindowManager *win = CEGUI::WindowManager::getSingletonPtr();
	CEGUI::Window *sheet = win->createWindow("DefaultGUISheet", "Root");

	m_pMainmenu = win->createWindow("TaharezLook/FrameWindow", "Root/Mainmenu");
	m_pMainmenu->setSize( CEGUI::UVector2(CEGUI::UDim(0.3, 0), CEGUI::UDim(0.3, 0)) );
	m_pMainmenu->setPosition( CEGUI::UVector2(CEGUI::UDim(0.35,0), CEGUI::UDim(0.35,0)) );
	sheet->addChildWindow(m_pMainmenu);

	m_pInsanityBar = (CEGUI::ProgressBar *)win->createWindow("TaharezLook/ProgressBar", "Root/InsanityBar");
	m_pInsanityBar->setSize( CEGUI::UVector2(CEGUI::UDim(0.6, 0), CEGUI::UDim(0.03, 0)) );
	m_pInsanityBar->setPosition( CEGUI::UVector2(CEGUI::UDim(0.2,0), CEGUI::UDim(0.01,0)) );
	sheet->addChildWindow(m_pInsanityBar);

	m_pMessageBox = win->createWindow("TaharezLook/FrameWindow", "Root/MessageBox");
	m_pMessageBox->setSize( CEGUI::UVector2(CEGUI::UDim(0.6, 0), CEGUI::UDim(0.1, 0)) );
	m_pMessageBox->setPosition( CEGUI::UVector2(CEGUI::UDim(0.2,0), CEGUI::UDim(0.05,0)) );
	sheet->addChildWindow(m_pMessageBox);

	m_pMessageBoxText = win->createWindow("TaharezLook/StaticText", "Root/MessageBox/Text" );
	m_pMessageBoxText->setSize( CEGUI::UVector2(CEGUI::UDim(0.9, 0), CEGUI::UDim(0.7, 0)) );
	m_pMessageBoxText->setPosition(CEGUI::UVector2(CEGUI::UDim(0.05, 0), CEGUI::UDim(0.25, 0)));
	m_pMessageBox->addChildWindow(m_pMessageBoxText);

	CEGUI::Window *load = win->createWindow("TaharezLook/Button", "Root/Mainmenu/LoadButton");
	load->setText("Tutorial");
	load->setSize(CEGUI::UVector2(CEGUI::UDim(0.6, 0), CEGUI::UDim(0.2, 0)));
	load->setPosition(CEGUI::UVector2(CEGUI::UDim(0.2, 0), CEGUI::UDim(0.15, 0)));
	load->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::LoadLevel, this));
	m_pMainmenu->addChildWindow(load);

	m_pAdventure = win->createWindow("TaharezLook/Button", "Root/Mainmenu/AdventureButton");
	m_pAdventure->setText("Adventure mode");
	m_pAdventure->setSize(CEGUI::UVector2(CEGUI::UDim(0.6, 0), CEGUI::UDim(0.2, 0)));
	m_pAdventure->setPosition(CEGUI::UVector2(CEGUI::UDim(0.2, 0), CEGUI::UDim(0.45, 0)));
	m_pAdventure->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::AdventureMode, this));
	m_pAdventure->disable();
	m_pMainmenu->addChildWindow(m_pAdventure);

	CEGUI::Window *quit = win->createWindow("TaharezLook/Button", "Root/Mainmenu/QuitButton");
	quit->setText("Quit game");
	quit->setSize(CEGUI::UVector2(CEGUI::UDim(0.6, 0), CEGUI::UDim(0.2, 0)));
	quit->setPosition(CEGUI::UVector2(CEGUI::UDim(0.2, 0), CEGUI::UDim(0.75, 0)));
	quit->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&CGameEngine::Quit, this));
	m_pMainmenu->addChildWindow(quit);

	m_pCounterbox = win->createWindow("TaharezLook/Listbox", "Root/Counter" );
	m_pCounterbox->setSize( CEGUI::UVector2(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.05, 0)) );
	sheet->addChildWindow(m_pCounterbox);

	m_pTorchescounter = win->createWindow("TaharezLook/StaticText", "Root/Counter/TorchesCounter" );
	m_pTorchescounter->setSize( CEGUI::UVector2(CEGUI::UDim(0.9, 0), CEGUI::UDim(0.8, 0)) );
	m_pTorchescounter->setPosition(CEGUI::UVector2(CEGUI::UDim(0.05, 0), CEGUI::UDim(0.1, 0)));
	m_pTorchescounter->setText("Torches: 10");
	m_pCounterbox->addChildWindow(m_pTorchescounter);

	m_pCounterbox->setVisible(false);
	m_pMessageBox->setVisible(false);
	m_pInsanityBar->setVisible(false);

	m_pGUISystem->setGUISheet(sheet);

	m_pMapLoader = new CMapLoader();
	return true;
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
	}

	WindowEventUtilities::removeWindowEventListener(m_pWindow, this);
	windowClosed(m_pWindow);

    m_pRoot = NULL;
    m_pCamera = NULL;
	m_pCameraNode = NULL;
    m_pSceneManager = NULL;
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

