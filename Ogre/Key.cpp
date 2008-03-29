/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#include "Key.h"
using namespace Ogre;

CKey::CKey(Entity *pEntity, SceneNode *pNode)
{
	m_pEntity = pEntity;
	m_pNode = pNode;
	m_bPickedUp = false;
}

bool CKey::frameStarted(const FrameEvent &evt)
{
	// Fancy rotating key!
	m_pNode->yaw( Radian(0.003f) );
	return true;
}

SceneNode* CKey::getSceneNode()
{
	return m_pNode;
}
