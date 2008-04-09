/************************************************************************************
*	Assignment 5 - The maze of despair												*
*	Tim Toxopeus - 3206947															*
*	Cetin Demir - 3236943															*
************************************************************************************/
#include "NameGenerator.h"

#include "string.h"

int CNameGenerator::CharacterIndex( char character )
{
	int index = 0;
	if ( character >= 97 && character <= 122 )
		index = character - 97;
	if ( character == '\'' )
		index = 26;
	return index;
}

char CNameGenerator::IndexCharacter( int index )
{
	if ( index < 0 )
	{
		index = index;
	}
	if ( index < 26 )
		return (char)(index + 97);
	return '\'';
}

bool CNameGenerator::IsVowel( char input )
{
	if ( input == 'a' || input == 'e' || input == 'i' || input == 'o' || input == 'u' || input == 'y' )
		return true;
	return false;
}

bool CNameGenerator::IsAcceptable( char *input )
{
	char prevChar = 0;
	int row = 1;

	for ( int i = 0; i<strlen(input); i++ )
	{
		if ( input[i] != prevChar )
		{
			prevChar = input[i];
			row = 1;
		}
		else
		{
			row++;
			if ( row >= 3 )
				return false;
		}
	}
	row = 0;
	for ( int i = 0; i<strlen(input); i++ )
	{
		if ( IsVowel(input[i]) )
		{
			row++;
			if ( row >= 3 )
				return false;
		}
		else
		{
			row = 0;
		}
	}
	row = 0;
	for ( int i = 0; i<strlen(input); i++ )
	{
		if ( !IsVowel(input[i]) )
		{
			row++;
			if ( row >= 3 )
				return false;
		}
		else
		{
			row = 0;
		}
	}
	return true;
}
