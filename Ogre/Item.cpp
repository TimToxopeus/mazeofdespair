/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#include "Item.h"
using namespace Ogre;

CLevelItem::CLevelItem(Entity *pEntity, SceneNode *pNode, int iTileX, int iTileY )
{
	m_pEntity = pEntity;
	m_pNode = pNode;
	m_iTileX = iTileX;
	m_iTileY = iTileY;
}

bool CLevelItem::frameStarted(const FrameEvent &evt)
{
	m_pNode->yaw( Radian(0.003f) );
	return true;
}
