/************************************************************************************
*	Assignment 4 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#include "GameEngine.h"
#include "windows.h"

// Program entry
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT cmdShow )
{
    // Create application object
	CGameEngine *game = CGameEngine::Instance();

	if ( game->Init() )
	{
		if ( game->Load() )
		{
			game->Run();
			game->Clean();
			game->Shutdown();
		}
	}

	delete game;

    return 0;
}
