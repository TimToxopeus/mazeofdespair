/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#include "MapLoader.h"

#include <iostream>
using namespace std;

#include "OgreLog.h"

#include "GameEngine.h"
#include "Tokenizer.h"
#include "Door.h"

#include "Generator.h"

extern std::string itoa2(const int x);

struct lightdata
{
	int strength;
	Vector3 diffuse;
	Vector3 specular;
};

CMapLoader::CMapLoader()
{
}

CMapLoader::~CMapLoader()
{
}

vector<string> CMapLoader::LoadMap(string mapfile, SceneManager *pManager, Camera *pCamera, bool generate)
{
	int l, k;
	vector<string> keylights;

	w = h = 15;

	// Check if the wallmesh has not yet been created
	if ( MeshManager::getSingleton().getByName("wallmesh").isNull() )
	{
		// Make my own cube, without top/bottom
		ManualObject mo("goodcube");
		mo.begin("walls/funkywall1");

		// Face1
			mo.position( 0, 0, 0 );
			mo.textureCoord(0, 0);
			mo.normal( -1, 0, 0 );

			mo.position( 0, 100, 0 );
			mo.textureCoord(0, 1);
			mo.normal( -1, 0, 0 );

			mo.position( 0, 0, 100 );
			mo.textureCoord(0, 1);
			mo.normal( -1, 0, 0 );

			mo.position( 0, 100, 100 );
			mo.textureCoord(1, 1);
			mo.normal( -1, 0, 0 );

			mo.triangle( 3, 1, 0 );
			mo.triangle( 0, 2, 3 );

		// Face2
			mo.position( 0, 0, 100 );
			mo.textureCoord(0, 0);
			mo.normal( 0, 0, 1 );

			mo.position( 0, 100, 100 );
			mo.textureCoord(0, 1);
			mo.normal( 0, 0, 1 );

			mo.position( 100, 0, 100 );
			mo.textureCoord(1, 0);
			mo.normal( 0, 0, 1 );

			mo.position( 100, 100, 100 );
			mo.textureCoord(1, 1);
			mo.normal( 0, 0, 1 );

			mo.triangle( 7, 5, 4 );
			mo.triangle( 4, 6, 7 );

		// Face3
			mo.position( 100, 0, 100 );
			mo.textureCoord(0, 0);
			mo.normal( 1, 0, 0 );

			mo.position( 100, 100, 100 );
			mo.textureCoord(0, 1);
			mo.normal( 1, 0, 0 );

			mo.position( 100, 0, 0 );
			mo.textureCoord(1, 0);
			mo.normal( 1, 0, 0 );

			mo.position( 100, 100, 0 );
			mo.textureCoord(1, 1);
			mo.normal( 1, 0, 0 );

			mo.triangle( 11, 9, 8 );
			mo.triangle( 8, 10, 11 );

		// Face4
			mo.position( 100, 0, 0 );
			mo.textureCoord(0, 0);
			mo.normal( 0, 0, -1 );

			mo.position( 100, 100, 0 );
			mo.textureCoord(0, 1);
			mo.normal( 0, 0, -1 );

			mo.position( 0, 0, 0 );
			mo.textureCoord(1, 0);
			mo.normal( 0, 0, -1 );

			mo.position( 0, 100, 0 );
			mo.textureCoord(1, 1);
			mo.normal( 0, 0, -1 );

			mo.triangle( 15, 13, 12 );
			mo.triangle( 12, 14, 15 );

		mo.end();
		mo.convertToMesh("wallmesh");
		// end cube
	}

	// If there already is cube data, clean it up
	if ( cubes.size() > 0 )
	{
		for ( int i = 0; i<cubes.size(); i++ )
			delete cubes[i];
	}
	cubes.clear();

	if ( generate )
	{
		// Generate a new maze
		w = h = atoi(mapfile.c_str());

		CGenerator *pGen = new CGenerator();
		pGen->GenerateMaze( w, h );
		pGen->BuildLevel( pManager, pCamera );

		cubes = pGen->getCubes();
		keylights = pGen->getKeylights();
		pGen->getNinja( ninjaX, ninjaY );

		delete pGen;

		// Increase width and height to correct plane creation and collision checks
		w *= 3; h *= 3;
	}
	else
	{
		// Open the map file
		fstream fin(mapfile.c_str());

		// Read width, height and light data
		char input[8];
		fin.getline(input, 8); // Read height
		h = atoi(input);
		fin.getline(input, 8); // Read width
		w = atoi(input);
		fin.getline(input, 8); // Read light count and keylight number

		CTokenizer t;
		vector<string> tokens = t.GetTokens(input);
		l = atoi(tokens[0].c_str());
		k = atoi(tokens[1].c_str());

		lightdata *lights = new lightdata[l];

		// Read individual light data
		for ( int i = 0; i < l; i++ )
		{
			char in[32];
			fin.getline( in, 32 ); // Read light data
			tokens = t.GetTokens( in, " " );
			lights[i].diffuse = Vector3( atoi(tokens[0].c_str()), atoi(tokens[1].c_str()), atoi(tokens[2].c_str()) );
			lights[i].specular = Vector3( atoi(tokens[3].c_str()), atoi(tokens[4].c_str()), atoi(tokens[5].c_str()) );
			lights[i].strength = atoi(tokens[6].c_str());
		}

		// Read map data
		int a, b;
		a = b = 0;
		for ( int y = 0; y<h; y++ )
		{
			for ( int x = 0; x<w; x++ )
			{
				char in;
				in = fin.get();

				if ( in == '#' ) // If the current character is a cube spawn a cube
				{
					char cubename[24];
					sprintf( cubename, "Cube%d", a );

					// Spawn cube at x,y
					Entity *ent1 = pManager->createEntity( cubename, "wallmesh" );
					ent1->setMaterialName("walls/funkywall1");

					sprintf( cubename, "Cube%d_node", a );
					SceneNode *node = pManager->getRootSceneNode()->createChildSceneNode( cubename );
					node->attachObject( ent1 );
					node->translate( Vector3(x * 100 - 50, -50, y * 100 - 50) );
					a++;

					// Push cubes into the cube stack
					cube *pNewCube = new cube();
					pNewCube->x = x;
					pNewCube->y = y;
					cubes.push_back(pNewCube);
				}
				if ( in == '_' ) // If the current character is a door spawn a door
				{
					// Spawn door at x,y
					Entity *ent1 = pManager->createEntity( "Door", "ninja.mesh" );
					ent1->setCastShadows(true);					

					SceneNode *node = pManager->getRootSceneNode()->createChildSceneNode( "Door_Node" );
					node->attachObject( ent1 );
					node->translate( Vector3(x * 100, -50, y * 100) );
					node->scale( Vector3( 0.5, 0.4, 0.5) );
					a++;

					ninjaX = x;
					ninjaY = y;

					pDoor = new CDoor( ent1, node );
					CGameEngine::Instance()->AddDoor( pDoor );
				}
				if ( in == 'K' ) // If the current character is a key spawn a key
				{
					// Spawn key at x,y
					Entity *ent1 = pManager->createEntity( "Key", "ogrehead.mesh" );
					ent1->setCastShadows(true);

					SceneNode *node = pManager->getRootSceneNode()->createChildSceneNode( "Key_Node" );
					node->attachObject( ent1 );
					node->translate( Vector3(x * 100, -20, y * 100) );
					node->scale( Vector3( 0.5, 0.5, 0.5) );

					keyX = x;
					keyY = y;

					pKey = new CKey( ent1, node );
					CGameEngine::Instance()->AddKey( pKey );
				}
				if ( in == 'L' ) // If the current character is a light switch spawn a light switch
				{
					// Spawn light switch at x,y
					Entity *ent1 = pManager->createEntity( "Switch", "robot.mesh" );
					ent1->setCastShadows(true);
					ent1->setMaterialName("robot/robot");
						
					SceneNode *node = pManager->getRootSceneNode()->createChildSceneNode( "Switch_Node" );
					node->attachObject( ent1 );
					node->translate( Vector3(x * 100, -20, y * 100) );
					node->scale( Vector3( 0.2, 0.2, 0.2) );
					
					CSwitch *pSwitch = new CSwitch( ent1, node );
					CGameEngine::Instance()->AddSwitch( pSwitch );
				}
				if ( in == 'X' ) // If the current character is a starting position move the camera there
				{
					pCamera->setPosition( x * 100, 0, y * 100 );
				}
				if ( (in >= 49 && in <= 57) && (in - 48) <= l ) // If the current character is a valid light ID number place a light
				{
					int index = in - 49;
					char lightname[24];
					sprintf( lightname, "Light%d", b );
					Light *light = pManager->createLight(lightname);
					light->setType(Light::LT_POINT);
					light->setPosition(Vector3((x * 100), 30, (y * 100)));
					light->setDiffuseColour(lights[index].diffuse.x,lights[index].diffuse.y,lights[index].diffuse.z);
					light->setSpecularColour(lights[index].specular.x,lights[index].specular.y,lights[index].specular.z);
					light->setAttenuation( lights[index].strength, 1.0f, 0.005f, 0.0f );
					b++;

					// If the light is a keylight, turn it off and push it into the keylights stack
					if ( index == k - 1 )
					{
						light->setVisible(false);
						string name = lightname;
						keylights.push_back(name);
					}
				}
			}
			fin.get();
		}

		fin.close();
	}

	// If there already are a ceiling and floor mesh, destroy their meshes
	if ( !MeshManager::getSingleton().getByName("ground").isNull() )
	{
		MeshManager::getSingleton().remove("ground");
		MeshManager::getSingleton().remove("ceiling");
	}

	// Create floor and ceiling plane mesh
	Plane plane(Vector3::UNIT_Y, -50);
	Plane plane2(Vector3::NEGATIVE_UNIT_Y, -50);
	MeshManager::getSingleton().createPlane("ground",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
		w * 100,h * 100,w,h,true,1,5,5,Vector3::UNIT_Z);
	MeshManager::getSingleton().createPlane("ceiling",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane2,
		w * 100,h * 100,w,h,true,1,5,5,Vector3::UNIT_Z);

	// Create the physical floor and ceiling plane object
	Entity *ent = pManager->createEntity("GroundEntity", "ground");
	SceneNode *groundPlane = pManager->getRootSceneNode()->createChildSceneNode();
	groundPlane->attachObject(ent);
	ent->setMaterialName("Examples/Rockwall");
	ent->setCastShadows(false);
	groundPlane->translate( Vector3( (w - 1) * 50, 0, (h - 1) * 50 ) );

	ent = pManager->createEntity("CeilingEntity", "ceiling");
	SceneNode *ceilingPlane = pManager->getRootSceneNode()->createChildSceneNode();
	ceilingPlane->attachObject(ent);
	ent->setMaterialName("Examples/Rockwall");
	ent->setCastShadows(false);
	ceilingPlane->translate( Vector3( (w - 1) * 50, 0, (h - 1) * 50 ) );

	// Return the keylights
	return keylights;
}

bool CMapLoader::hasCube(Ogre::Vector3 position, bool ignoreNinja)
{
	if ( cubes.size() == 0 )
		return false;

	// Calculate the position on a tile by clipping it
	int clippedX, clippedY;
	clippedX = (position.x + 50 - (((int)position.x + 50) % 100)) / 100;
	clippedY = (position.z + 50 - (((int)position.z + 50) % 100)) / 100;

	// Ensure that the camera is not trying to move outside the game world
	if ( clippedX >= 0 && clippedX < w )
	{
		if ( clippedY >= 0 && clippedY < h )
		{
			// The camera cannot move through an unlocked door
			if ( clippedX == ninjaX && clippedY == ninjaY )
			{
				if ( pDoor )
				{
					if ( pDoor->IsUnlocked() == false && !ignoreNinja )
						return true;
				}
				else
				{
					pDoor = CGameEngine::Instance()->GetDoor(0);
					if ( pDoor->IsUnlocked() == false && !ignoreNinja )
						return true;
				}
			}

			// Check all cubes, if there is a cube in the specified position return true
			for ( int a = 0; a<cubes.size(); a++ )
			{
				if ( cubes[a]->x == clippedX && cubes[a]->y == clippedY )
				{
					return true;
				}
			}
		}
		else // Assume the outside world is massive
			return true;
	}
	else // Assume the outside world is massive
		return true;

	// Nothing is found
	return false;
}
