/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

#include "NameGenerator.h"

class CDoubleMarkov : public CNameGenerator
{
protected:
	int freqTable[756][28];

	void ParseName( char name[] );
	char GetCharacterFromSet( char firstChar, char secondChar );

	int GetIndex( char firstChar, char secondChar );

public:
	void GenerateTable();
	virtual void GenerateName( int length, char *name );
};
