/************************************************************************************
*	Assignment 4 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

#include "Ogre.h"
using namespace Ogre;

#include <string>
#include <vector>

#include "Tile.h"
#include "Door.h"
#include "Key.h"

class CMapLoader
{
private:
	int w, h;
	std::vector<cube *> cubes;

	int ninjaX, ninjaY, keyX, keyY;
	CDoor *pDoor;
	CKey *pKey;

public:
	CMapLoader();
	~CMapLoader();

	// LoadMap loads the map
	// If generate is false or not supplied, mapfile contains the name of the text file containing the level data
	// If generate is true, mapfile contains the size of the map to be generated
	std::vector<std::string> LoadMap( std::string mapfile, SceneManager *pManager, Camera *pCamera, bool generate = false );

	// hasCube returns true when there is a cube at the requested position.
	// This is used in a fast look-up collision detection.
	bool hasCube( Ogre::Vector3 position, bool ignoreNinja = false );
};
