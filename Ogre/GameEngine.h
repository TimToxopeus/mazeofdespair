/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

#include <CEGUI/CEGUI.h>
#include <OgreCEGUIRenderer.h>

//Use this define to signify OIS will be used as a DLL
//(so that dll import/export macros are in effect)
#define OIS_DYNAMIC_LIB
#include <OIS/OIS.h>

using namespace Ogre;

#include "MapLoader.h"
#include "Door.h"
#include "Key.h"
#include "Switch.h"

#include "Player.h"
#include "Combatant.h"
#include "CombatMode.h"
#include "RandomizedFactory.h"

#define CAMERA_NAME "SceneCamera"

enum GUIMode
{
	NONE,
	MAINMENU,
	CITY,
	ADVENTURE,
	BATTLEMODEMENU,
	BATTLEMODEATTACK
};

class CGameEngine : public FrameListener, WindowEventListener, public OIS::MouseListener
{
private:
    Root *m_pRoot;
    Camera *m_pCamera;						// The camera object pointer
	Camera *m_pCombatCamera;				// The camera object pointer
	SceneNode *m_pCameraNode;
    SceneManager *m_pPrimary;				// The Primary Scene Manager object pointer
	SceneManager *m_pSecondary;				// The Secondary Scene Manager object pointer
    RenderWindow *m_pWindow;				
	RaySceneQuery *m_pRaySceneQuery;		// The ray scene query pointer

	//OIS Input devices
	OIS::InputManager *m_pInputManager;
	OIS::Mouse *m_pMouse;					// The mouse object pointer
	OIS::Keyboard *m_pKeyboard;				// The keyboard object pointer
	OIS::JoyStick *m_pJoy;					// The joystick object pointer

	// Movement
	Vector3 m_vTranslateVector;				// The translate vector for camera movement
	float m_fMoveScale;
	Degree m_fRotScale;
	Real m_fMoveSpeed;
	Degree m_fRotateSpeed;

	Real m_fFrameTime;

	// Other
	Light *m_pFlashlight;					// The two flashlights of the player
	Light *m_pFlashlight2;
	
	bool m_bKeySelected;					// The boolean for checking if the player selected the key
	bool mLMouseDown, mRMouseDown;			// Booleans for mouse down events.
	TextureFilterOptions m_eFiltering;
	int m_iAniso;
	int m_iSceneDetailIndex ;				
	Overlay* m_pDebugOverlay;				// The Debug overlay.
	bool m_bStatsOn;						// Boolean for setting the Stats on screen.
	Real m_fTimeUntilNextToggle;			// Timer for toggling.
	unsigned int m_iNumScreenShots;			// Counter for screenshots.
	std::string m_szDebugText;				// String for debug text on screen.
	std::vector<std::string> keylights;		// Vector that keeps track of the keylights
	bool m_bQuit;							// Boolean for when the player quits the game.
	bool m_bLoaded;							// Boolean for level loaded.
	int m_iLevel;							// Level 
	int m_iTorchCount;						// Counter for torches.
	int m_iPlacedTorches;					// Counter for torches placed.
	Real m_fInsanityTimeLeft;				// Timer for insanity function.
	CMapLoader *m_pMapLoader;				// The maploader object
	bool m_bInAdventureMode;				// If the player is in adventure mode, this boolean is true.
	bool m_bLevelComplete;					// The boolean for completing a level. 
	bool m_bDisplayedSwitchTip;				// The boolean for displaying the light switch tip in tutorial mode. 
	bool m_bMapVisible;						// The boolean for displaying the map

	CPlayer *m_pPlayer;
	CRandomizedFactory *m_pFactory;
	bool m_bHadCityOpen;
	int m_iSelectedType;
	int m_iSelectedIndex;

	static CGameEngine *m_pInstance;		// Singleton instance of the CGameEngine object.
	CGameEngine();				

	// Objects - Vectors that hold the objects.
	std::vector<CDoor *> m_pDoors;
	std::vector<CKey *> m_pKeys;
	std::vector<CSwitch *> m_pSwitches;
	std::vector<CItem *> m_pCityItems;
	std::vector<CLevelItem *> m_pLevelItems;
	std::vector<CMonster *> m_pMonsters;

	// Map variables
	std::vector<cube *> m_pHiddenCubes;
	std::vector<cube *> m_pDiscoveredCubes;
	int m_iOldClippedX;
	int m_iOldClippedY;

	// CEGUI
	CEGUI::System *m_pGUISystem;
	CEGUI::OgreCEGUIRenderer *m_pGUIRenderer;

	// Window objects for all the menu's and bars.
	CEGUI::WindowManager *m_pWindowManager;
	CEGUI::Window *m_pSheet;
	CEGUI::Window *m_pCity;
	CEGUI::Window *m_pEasyLevel;
	CEGUI::Window *m_pMediumLevel;
	CEGUI::Window *m_pHardLevel;
	CEGUI::Window *m_pChallengingLevel;
	CEGUI::Window *m_pImpossibleLevel;
	CEGUI::Window *m_pItems;
	CEGUI::Window *m_pEquipment;
	CEGUI::Window *m_pInventory;
	CEGUI::Window *m_pMainmenu;	
	CEGUI::Window *m_pCounterbox;
	CEGUI::Window *m_pTorchescounter;
	CEGUI::Window *m_pMessageBox;
	CEGUI::Window *m_pMessageBoxText;
	CEGUI::ProgressBar *m_pInsanityBar;
	CEGUI::Window *m_pAdventure;
	CEGUI::Window *m_pItemLabel;
	CEGUI::Window *m_pItemWindow;
	CEGUI::Window *m_pCharacterWindow;
	CEGUI::Window *m_pBuySellSelected;
	CEGUI::Window *m_pEquipDequipSelected;
	CEGUI::Window *m_pMap;
	CEGUI::Window *m_pMapPlayer;
	CEGUI::Imageset *m_pMapWallSet;

	// Window objects for the combat mode.
	CEGUI::Window *m_pCombatWindow;
	CEGUI::Window *m_pAttackWindow;
	CEGUI::Window *m_pCombatBars;
	CEGUI::ProgressBar *m_pPlayerHealthBar;	
	CEGUI::ProgressBar *m_pPlayerRageBar;	
	CEGUI::ProgressBar *m_pEnemyHealthBar;
	CEGUI::Window *m_pCombatText;
	CEGUI::Window *m_pCombatMenu;
	CEGUI::Window *m_pAttackMenu;
	
	CEGUI::Window *m_pThunderButton;
	CEGUI::Window *m_pDoubleButton;
	
	// Combat stuff
	Ogre::Vector3 m_vCameraPos;
	bool m_bInCombatMode;
	CombatMode *pCombatMode;
	Light *m_pCombatLight;

	// Timer for windows.
	Real m_fMessageTime;

public:
	static CGameEngine *Instance() { if ( !m_pInstance ) m_pInstance = new CGameEngine(); return m_pInstance; }
	
	// Initialize 
	bool Init();
	// Load map and objects
	bool Load();

	void SetupViewport(RenderWindow *pWindow, SceneManager *pCurrent);
	static void Swap(SceneManager *&first, SceneManager *&second);
	void SetCameraPosition(Vector3 pVec);

	// Function that loads and sets up the tutorial level.
	bool LoadLevel( const CEGUI::EventArgs &e );
	// Function that loads and sets up adventure mode.
	bool AdventureMode( const CEGUI::EventArgs &e );
	// Quit the game
	bool Quit( const CEGUI::EventArgs &e );
	
	// Start the rendering.
	void Run();

	void ShowCity(bool refreshShop = true);
	bool ClickItem( const CEGUI::EventArgs &e );
	bool BuySellItem( const CEGUI::EventArgs &e );
	bool EquipDequipItem( const CEGUI::EventArgs &e );
	bool NextLevel( const CEGUI::EventArgs &e );										// Function that sets up the next level.
	void ContinueGame(CCombatant *pActualMonster, CMonster *pMonster);					//Continues the game after combat.

	void Clean();
	void Shutdown();

	// Input
	virtual bool processUnbufferedKeyInput(const FrameEvent& evt);

	// FrameListener
	virtual bool frameStarted(const FrameEvent& evt);
	virtual bool frameEnded(const FrameEvent& evt);

	// WindowEventListener
	virtual void windowResized(RenderWindow* rw);
	virtual void windowClosed(RenderWindow* rw);

	// Other
	// Move the camera, but check for collisions
	void moveCamera();
	void showDebugOverlay(bool show);
	void updateStats(void);
	// Toggle the lights
	void toggleLights();
	void AddDoor( CDoor *pDoor ) { m_pDoors.push_back( pDoor ); }
	void AddKey( CKey *pKey ) { m_pKeys.push_back( pKey ); }
	void AddSwitch( CSwitch *pSwitch ) { m_pSwitches.push_back( pSwitch ); }
	void AddItem( CLevelItem *pItem ) { m_pLevelItems.push_back( pItem ); }
	void AddMonster( CMonster *pMonster ) { m_pMonsters.push_back( pMonster ); }
	SceneManager *GetSceneManager() { return m_pPrimary; }
	CDoor *GetDoor( int index ) { if ( index >= 0 && index < m_pDoors.size() ) return m_pDoors[index]; return NULL; }

	// GUI
	CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID);
	void SetGUIMode( GUIMode mode );
	void SetCombatText ( CEGUI::String pText );
	void UpdateRageButtons();

	// Mouse events
	bool mouseMoved(const OIS::MouseEvent &arg);
	bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	void onLeftPressed(const OIS::MouseEvent &arg);
	void onRightPressed(const OIS::MouseEvent &arg);
};
