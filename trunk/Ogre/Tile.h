/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

// The tile object contains information for a map tile
// This is used in map generation.
class tile
{
public:
	tile() { u = d = l = r = 1; camera = key = door = light = item = monster = false; }
	int u, d, l, r, x, y, i;
	bool camera;
	bool key;
	bool door;
	bool light;
	bool item;
	bool monster;
	bool fullyIntact() { if ( u == d && u == l && u == r && u == 1 ) return true; return false; }
	void setXY( int x, int y ) { this->x = x; this->y = y; }

	// openSides returns the amount of open sides for this tile. Used in calculating if this is an intersection or a path.
	int openSides() { int i = 0; if ( d == 0 ) i++; if ( u == 0 ) i++; if ( l == 0 ) i++; if ( r == 0 ) i++; return i; }
};

// Simple cube object, used in look up tests for collision detection
class cube
{
public:
	cube() { x = y = -1; }
	int x, y;
};