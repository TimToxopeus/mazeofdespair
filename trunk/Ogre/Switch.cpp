/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#include "Switch.h"
using namespace Ogre;

CSwitch::CSwitch(Entity *pEntity, SceneNode *pNode)
{
	m_pEntity = pEntity;
	m_pNode = pNode;
}

bool CSwitch::frameStarted(const FrameEvent &evt)
{
	// Bouncy, rotating light switch!
	m_pNode->yaw( Radian(0.003f) );
	Radian yaw = m_pNode->getOrientation().getYaw();
	Vector3 pos = m_pNode->getPosition();
	pos.y += Ogre::Math::Sin( yaw ) / 20;
	m_pNode->setPosition( pos );
	return true;
}
