/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

#include "Ogre.h"
using namespace Ogre;

#include "Tile.h"

// Settings structure, to store information of the generated level
// This is used to determine positions for the start, end, key and light objects.
struct settings
{
	int start, end, key, light;
	int value;
};

// Generator class
class CGenerator
{
private:
	tile *tiles;
	int w, h;
	int **distancetable; // The distance table stores distances between end-nodes.

	std::vector<cube *> cubes; // A vector storing all cubes, used in collision detection
	std::vector<std::string> keylights; // A vector storing all keylights, used in lighting
	int ninjaX, ninjaY; // X/Y coordinates of the ninja (door)

	int cubeCount;
	void PlaceCube( SceneManager *pManager, int x, int y ); // Places a cube in the scene

	int getDistanceFromTable( int a, int b );
	int getDistance(int x, int y, int x2, int y2, int prevX, int prevY); // Calculate distance between two nodes, uses Depth First Algorithm
	int PlaceKeylights( int x, int y, int x2, int y2, int prevX, int prevY ); // Places keylights at all intersections between the supplied nodes. (Key and Light)
	void SpawnLight( int x, int y ); // Spawns a light

public:
	CGenerator() { tiles = 0; cubeCount = w = h = 0; distancetable = 0; }
	~CGenerator() { CleanUp(); }

	void CleanUp(); // Clean up memory used by generator
	void GenerateMaze( int w, int h ); // Generate a maze
	void BuildLevel( SceneManager *pManager, Camera *pCamera ); // Build scene based on generated level
	std::vector<cube *> getCubes() { return cubes; }
	std::vector<std::string> getKeylights() { return keylights; }
	void getNinja( int &x, int &y ) { x = ninjaX * 3 + 1; y = ninjaY * 3 + 1; }
};
