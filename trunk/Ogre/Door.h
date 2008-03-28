/************************************************************************************
*	Assignment 4 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

#include "Ogre.h"
using namespace Ogre;

class CDoor
{
private:
	Entity *m_pEntity;
	SceneNode *m_pNode;

	bool m_bUnlocked;
	Real m_fRotation;

public:
	CDoor( Entity *pEntity, SceneNode *pNode );
	bool frameStarted( const FrameEvent &evt );
	void Unlock() { m_bUnlocked = true; }
	bool IsUnlocked() { return m_bUnlocked; }
	SceneNode* getSceneNode();
};
