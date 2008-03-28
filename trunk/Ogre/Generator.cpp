/************************************************************************************
*	Assignment 4 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#include "Generator.h"
#include <vector>
using namespace std;

#include "GameEngine.h"
#include "Door.h"
#include "Key.h"

// Convert an integer into a std::string object
std::string itoa2(const int x)
{
  std::ostringstream o;
  if (!(o << x)) return "ERROR";
  return o.str();
}

void CGenerator::CleanUp()
{
	// Delete tiles
	if ( tiles != 0 )
	{
		delete [] tiles;
		tiles = 0;
	}

	// Delete distance table
	if ( distancetable )
	{
		for ( int i = 0; i<h; i++ )
			delete [] distancetable[i];
		delete distancetable;
		distancetable = 0;
	}
}

void CGenerator::GenerateMaze( int w, int h )
{
	vector<tile *> deadends;
	this->w = w;
	this->h = h;

	// There have to be atleast 4 deadends for placing the start, the ninja, the lightswitch and the key.
	while ( deadends.size() < 4 )
	{
		// Remove any old data, if it exists
		deadends.clear();
		CleanUp();

		// Create new tile array and initialize it's values
		tiles = new tile[w*h];
		for ( int a = 0; a<w*h; a++ )
		{
			tiles[a].setXY( a % h, a / h );
			tiles[a].i = a;
		}

		// Prepare algorithm variables for depth first algorithm
		int totalCells, visitedCells;
		totalCells = w * h;
		vector<tile *> cellStack;
		tile *currentCell;

		// Choose a random starting location
		int x = rand()%w;
		int y = rand()%h;
		visitedCells = 1;
		currentCell = &tiles[x + y * h];

		// While not all cells have been visited
		while ( visitedCells < totalCells )
		{
			// Start out with the current cell
			x = currentCell->x;
			y = currentCell->y;

			// Calculate its neighbours that have fully intact walls, ie; have not yet been visited
			vector<tile *> neighbours;
			if ( x > 0  )
			{
				int index = (x - 1) + y * h;
				if ( tiles[index].fullyIntact() )
					neighbours.push_back( &tiles[index] );
			}
			if ( x < (w - 1) )
			{
				int index = (x + 1) + y * h;
				if ( tiles[index].fullyIntact() )
					neighbours.push_back( &tiles[index] );
			}
			if ( y > 0 )
			{
				int index = x + (y - 1) * h;
				if ( tiles[index].fullyIntact() )
					neighbours.push_back( &tiles[index] );
			}
			if ( y < (h - 1) )
			{
				int index = x + (y + 1) * h;
				if ( tiles[index].fullyIntact() )
					neighbours.push_back( &tiles[index] );
			}

			// If more than 0 neighbours are found
			if ( neighbours.size() > 0 )
			{
				// Pick a random neighbour
				int rnd = rand()%(int)neighbours.size();
				tile *random = neighbours[rnd];

				// Break down the walls between the current cell and the chosen neighbour
				if ( random->x < currentCell->x )
				{
					currentCell->l = 0;
					random->r = 0;
				}
				else if ( random->x > currentCell->x )
				{
					currentCell->r = 0;
					random->l = 0;
				}
				else if ( random->y < currentCell->y )
				{
					currentCell->u = 0;
					random->d = 0;
				}
				else if ( random->y > currentCell->y )
				{
					currentCell->d = 0;
					random->u = 0;
				}

				// Push the current cell onto the cell stack and set the chosen neighbour as the current cell
				cellStack.push_back( currentCell );
				currentCell = random;
				
				// Increment cell counter
				visitedCells++;
			}
			else
			{
				// Pop an old cell of the cellstack; backtrace to the start.
				currentCell = cellStack.back();
				cellStack.pop_back();
			}
		}

		// Calculate the dead ends in the maze
		for ( int i = 0; i<w*h; i++ )
		{
			if ( tiles[i].openSides() == 1 )
			{
				deadends.push_back(&tiles[i]);
			}
		}
	}

	// Now it is time to build the distance table, to ensure a fairly even distribution between the start, end, key and light nodes through the maze.
	int decount = deadends.size();
	distancetable = new int*[decount];
	for ( int i = 0; i<decount; i++ )
	{
		distancetable[i] = new int[decount];
		for ( int j = 0; j<decount; j++ )
			distancetable[i][j] = -1;
		distancetable[i][i] = 0;
	}

	// Fill up the distance table with the distances between each dead end node to every other dead end node.
	for ( int i = 0; i<decount; i++ )
	{
		for ( int j = 0; j<decount; j++ )
		{
			if ( j == i )
				continue;

			if ( distancetable[i][j] == -1 )
			{
				distancetable[i][j] = distancetable[j][i] = getDistance( deadends[i]->x, deadends[i]->y, deadends[j]->x, deadends[j]->y, deadends[i]->x, deadends[i]->y );
			}
		}
	}

	// Calculate settings, determines the distance value of each possible node combination
	vector<settings> solutions;
	for ( int i = 0; i<decount; i++ )
	{
		settings setting;
		int a, b, c, dist;
		a = b = c = dist = setting.value = 0;

		setting.start = i;
		for ( int j = 0; j<decount; j++ )
		{
			if ( i == j )
				continue;

			setting.end = j;
			for ( int k = 0; k<decount; k++ )
			{
				if ( i == k || j == k )
					continue;

				setting.key = k;
				for ( int l = 0; l<decount; l++ )
				{
					if ( i == l || j == l || k == l )
						continue;

					setting.light = l;

					setting.value = distancetable[i][j] + distancetable[i][k] + distancetable[k][j];
					solutions.push_back( setting );
				}
			}
		}
	}

	// Find highest value solution
	int value = 0;
	vector<settings> top;
	int solution = 0;
	for ( int i = 0; i<solutions.size(); i++ )
	{
		if ( solutions[i].value > value )
		{
			value = solutions[i].value;
		}
	}
	for ( int i = 0; i<solutions.size(); i++ )
	{
		if ( solutions[i].value == value )
		{
			top.push_back( solutions[i] );
		}
	}
	int v = 0;
	int choice = 0;
	// Apply average distance calculation, solutions with the most average distance between nodes gets chosen
	for ( int i = 0; i<top.size(); i++ )
	{
		settings setting = top[i];
		int d = distancetable[setting.start][setting.end] +
			distancetable[setting.start][setting.key] +
			distancetable[setting.start][setting.light] +
			distancetable[setting.end][setting.key] +
			distancetable[setting.end][setting.light] +
			distancetable[setting.key][setting.light];
		d = d / 6;

		if ( d > v )
		{
			v = d;
			choice = i;
		}
	}

	// The nodes that receive the positions are now known, set these values
	deadends[top[choice].start]->camera = true;
	deadends[top[choice].end]->door = true;
	deadends[top[choice].key]->key = true;
	deadends[top[choice].light]->light = true;
}

void CGenerator::BuildLevel( SceneManager *pManager, Camera *pCamera )
{
	if ( tiles == 0 )
		return;

	// Loop through all the tiles
	for ( int y = 0; y<h; y++ )
	{
		// Each tile receives nine cube locations.	XXX
		//											X X
		//											XXX
		// So loop through the Y 3 additional times.
		for ( int l = 0; l<3; l++ )
		{
			for ( int x = 0; x<w; x++ )
			{
				tile *pTile = &tiles[x + y * h];

				// Place cubes
				// Check if the wall is open or not, if it is don't place a cube there
				if ( l == 0 )
				{
					PlaceCube( pManager, x * 3, y * 3 );
					if ( pTile->u != 0 )
						PlaceCube( pManager, x * 3 + 1, y * 3 );
					PlaceCube( pManager, x * 3 + 2, y * 3 );
				}
				if ( l == 1 )
				{
					if ( pTile->l != 0 )
						PlaceCube( pManager, x * 3, y * 3 + 1 );
					if ( pTile->r != 0 )
						PlaceCube( pManager, x * 3 + 2, y * 3 + 1 );
				}
				if ( l == 2 )
				{
					PlaceCube( pManager, x * 3, y * 3 + 2);
					if ( pTile->d != 0 )
						PlaceCube( pManager, x * 3 + 1, y * 3 + 2 );
					PlaceCube( pManager, x * 3 + 2, y * 3 + 2 );
				}
			}
		}
	}

	// Create key, door, lightswitch objects and position the camera
	int lightX, lightY, keyX, keyY;
	for ( int i = 0; i<w*h; i++ )
	{
		int x, y;
		x = ((tiles[i].x * 3) + 1) * 100;
		y = ((tiles[i].y * 3) + 1) * 100;
		if ( tiles[i].camera )
		{
			pCamera->setPosition( x, 0, y );
		}
		if ( tiles[i].key )
		{
			Entity *ent1 = pManager->createEntity( "Key", "ogrehead.mesh" );
			ent1->setCastShadows(true);

			SceneNode *node = pManager->getRootSceneNode()->createChildSceneNode( "Key_Node" );
			node->attachObject( ent1 );
			node->translate( Vector3(x, -20, y) );
			node->scale( Vector3( 0.5, 0.5, 0.5) );

			keyX = tiles[i].x;
			keyY = tiles[i].y;

			CKey *pKey = new CKey( ent1, node );
			CGameEngine::Instance()->AddKey( pKey );
		}
		if ( tiles[i].door )
		{
			Entity *ent1 = pManager->createEntity( "Door", "ninja.mesh" );
			ent1->setCastShadows(true);

			SceneNode *node = pManager->getRootSceneNode()->createChildSceneNode( "Door_Node" );
			node->attachObject( ent1 );
			node->translate( Vector3(x, -50, y) );
			node->scale( Vector3( 0.5, 0.4, 0.5) );

			ninjaX = tiles[i].x;
			ninjaY = tiles[i].y;

			CDoor *pDoor = new CDoor( ent1, node );
			CGameEngine::Instance()->AddDoor( pDoor );
		}
		if ( tiles[i].light )
		{
			Entity *ent1 = pManager->createEntity( "Switch", "robot.mesh" );
			ent1->setCastShadows(true);
			ent1->setMaterialName( "robot/robot" );

			SceneNode *node = pManager->getRootSceneNode()->createChildSceneNode( "Switch_Node" );
			node->attachObject( ent1 );
			node->translate( Vector3(x, -20, y) );
			node->scale( Vector3( 0.2, 0.2, 0.2) );

			lightX = tiles[i].x;
			lightY = tiles[i].y;

			CSwitch *pSwitch = new CSwitch( ent1, node );
			CGameEngine::Instance()->AddSwitch( pSwitch );
		}
	}

	// Place all the keylights, uses the same depth first search algorithm as the distance table calculation
	PlaceKeylights( lightX, lightY, keyX, keyY, lightX, lightY );
}

void CGenerator::PlaceCube(SceneManager *pManager, int x, int y)
{
	char cubename[24];
	sprintf( cubename, "Cube%d", cubeCount );
	Entity *ent1 = pManager->createEntity( cubename, "wallmesh" );
	ent1->setMaterialName("walls/funkywall1");

	sprintf( cubename, "Cube%d_node", cubeCount );
	SceneNode *node = pManager->getRootSceneNode()->createChildSceneNode( cubename );
	node->attachObject( ent1 );
	node->translate( Vector3(x * 100 - 50, -50, y * 100 - 50) );

	cubeCount++;

	cube *pNewCube = new cube();
	pNewCube->x = x;
	pNewCube->y = y;
	cubes.push_back( pNewCube );
}

int CGenerator::getDistanceFromTable(int a, int b)
{
	return distancetable[a][b];
}

int CGenerator::getDistance(int x, int y, int x2, int y2, int prevX, int prevY)
{
	// Depth first search algorithm
	if ( x == x2 && y == y2 )
		return 0;

	tile *pTile = &tiles[x + y * h];

	// Can search up, and did not come from there?
	if ( pTile->u == 0 && prevY != y - 1 )
	{
		// Get distance
		int d = getDistance( x, y - 1, x2, y2, x, y );
		if ( d != -1 ) // If a solution is found, increment distance by 1 and return
			return 1 + d;
	}
	// Can search down, and did not come from there?
	if ( pTile->d == 0 && prevY != y + 1 )
	{
		// Get distance
		int d = getDistance( x, y + 1, x2, y2, x, y );
		if ( d != -1 ) // If a solution is found, increment distance by 1 and return
			return 1 + d;
	}
	// Can search left, and did not come from there?
	if ( pTile->l == 0 && prevX != x - 1 )
	{
		// Get distance
		int d = getDistance( x - 1, y, x2, y2, x, y );
		if ( d != -1 ) // If a solution is found, increment distance by 1 and return
			return 1 + d;
	}
	// Can search right, and did not come from there?
	if ( pTile->r == 0 && prevX != x + 1 )
	{
		// Get distance
		int d = getDistance( x + 1, y, x2, y2, x, y );
		if ( d != -1 ) // If a solution is found, increment distance by 1 and return
			return 1 + d;
	}

	// No solution possible from this node!
	return -1;
}

int CGenerator::PlaceKeylights(int x, int y, int x2, int y2, int prevX, int prevY)
{
	// Depth first search algorithm
	if ( x == x2 && y == y2 )
		return 0;

	tile *pTile = &tiles[x + y * h];

	// Can search up, and did not come from there?
	if ( pTile->u == 0 && prevY != y - 1 )
	{
		// Get distance
		int d = PlaceKeylights( x, y - 1, x2, y2, x, y );
		if ( d != -1 ) // If a solution is found, increment distance by 1 and return
		{
			// Check if current tile has 3 or more open walls, if so place a keylight
			if ( pTile->openSides() >= 3 )
				SpawnLight( x, y );
			return 1 + d;
		}
	}
	// Can search down, and did not come from there?
	if ( pTile->d == 0 && prevY != y + 1 )
	{
		// Get distance
		int d = PlaceKeylights( x, y + 1, x2, y2, x, y );
		if ( d != -1 ) // If a solution is found, increment distance by 1 and return
		{
			// Check if current tile has 3 or more open walls, if so place a keylight
			if ( pTile->openSides() >= 3 )
				SpawnLight( x, y );
			return 1 + d;
		}
	}
	// Can search left, and did not come from there?
	if ( pTile->l == 0 && prevX != x - 1 )
	{
		// Get distance
		int d = PlaceKeylights( x - 1, y, x2, y2, x, y );
		if ( d != -1 ) // If a solution is found, increment distance by 1 and return
		{
			// Check if current tile has 3 or more open walls, if so place a keylight
			if ( pTile->openSides() >= 3 )
				SpawnLight( x, y );
			return 1 + d;
		}
	}
	// Can search right, and did not come from there?
	if ( pTile->r == 0 && prevX != x + 1 )
	{
		// Get distance
		int d = PlaceKeylights( x + 1, y, x2, y2, x, y );
		if ( d != -1 ) // If a solution is found, increment distance by 1 and return
		{
			// Check if current tile has 3 or more open walls, if so place a keylight
			if ( pTile->openSides() >= 3 )
				SpawnLight( x, y );
			return 1 + d;
		}
	}

	// No solution possible from this node!
	return -1;
}

void CGenerator::SpawnLight( int x, int y )
{
	char lightname[24];
	sprintf( lightname, "Light%d", keylights.size() );
	Light *light = CGameEngine::Instance()->GetSceneManager()->createLight(lightname);
	light->setType(Light::LT_POINT);
	light->setPosition(Vector3(((x * 3 + 1) * 100), 30, ((y * 3 + 1) * 100)));
	light->setDiffuseColour(0.0f, 0.0f, 1.0f);
	light->setSpecularColour(0.0f, 0.0f, 1.0f);
	light->setAttenuation( 500.0f, 1.0f, 0.005f, 0.0f );

	light->setVisible(false); // Keylights are turned off by default
	string name = lightname;
	keylights.push_back(name);
}
