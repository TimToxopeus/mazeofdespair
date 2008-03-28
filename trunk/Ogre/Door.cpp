/************************************************************************************
*	Assignment 4 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#include "Door.h"
using namespace Ogre;

CDoor::CDoor(Entity *pEntity, SceneNode *pNode)
{
	m_pEntity = pEntity;
	m_pNode = pNode;
	m_bUnlocked = false;
	m_fRotation = 0.0f;
}

bool CDoor::frameStarted(const FrameEvent &evt)
{
	// Play falling over animation when the door is unlocked.
	if ( m_bUnlocked && m_fRotation < 1.57f )
	{
		Real m_fRotate = 3 * evt.timeSinceLastFrame;
		Radian angle = Radian(m_fRotate);
		m_pNode->pitch( angle );
		m_fRotation += m_fRotate;
	}
	return true;
}

SceneNode* CDoor::getSceneNode()
{
	return m_pNode;
}
