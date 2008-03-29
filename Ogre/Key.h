/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

#include "Ogre.h"
using namespace Ogre;

class CKey
{
private:
	Entity *m_pEntity;
	SceneNode *m_pNode;
	bool m_bPickedUp;

public:
	CKey( Entity *pEntity, SceneNode *pNode );
	bool frameStarted( const FrameEvent &evt );
	SceneNode* getSceneNode();

	void Pickup() { m_bPickedUp = true; }
	void Drop() { m_bPickedUp = false; }
	bool IsPickedUp() { return m_bPickedUp; }
};
