/************************************************************************************
*	Assignment 4 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

#include "Ogre.h"
using namespace Ogre;

class CSwitch
{
private:
	Entity *m_pEntity;
	SceneNode *m_pNode;

public:
	CSwitch( Entity *pEntity, SceneNode *pNode );
	bool frameStarted( const FrameEvent &evt );
	SceneNode* getSceneNode() { return m_pNode; }
};
