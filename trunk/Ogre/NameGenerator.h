/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#pragma once

class CNameGenerator
{
protected:
	int CharacterIndex( char character );
	char IndexCharacter( int index );
	bool IsAcceptable( char *input );
	bool IsVowel( char input );

public:
	virtual void GenerateName( int length, char *name ) = 0;
};
