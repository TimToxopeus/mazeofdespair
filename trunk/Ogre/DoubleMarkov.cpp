#include "DoubleMarkov.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"

int CDoubleMarkov::GetIndex( char firstChar, char secondChar )
{
	int index = 0;

	int a = CharacterIndex( firstChar );
	int b = CharacterIndex( secondChar );

	index = a * 27 + b;

	return index + 28;
}

void CDoubleMarkov::ParseName( char *name )
{
	if ( strlen( name ) < 3 )
		return;

	char firstChar = name[0];
	char secondChar = name[1];
	char thirdChar = 0;

	freqTable[0][CharacterIndex(firstChar)] += 1;
	freqTable[1 + CharacterIndex(firstChar)][CharacterIndex(secondChar)] += 1;

	for ( int i = 2; i < (int)strlen(name); i++ )
	{
		thirdChar = name[i];
		freqTable[GetIndex(firstChar, secondChar)][CharacterIndex(thirdChar)] += 1;
		firstChar = secondChar;
		secondChar = thirdChar;
	}
}

void CDoubleMarkov::GenerateTable()
{
	char input[16];
	int i = 0;

	memset( freqTable, 0, 756 * 28 * sizeof(int) );

	FILE *pFile = fopen( "names.txt", "rt" );

	while ( !feof( pFile ) )
	{
		fgets( input, 16, pFile );
		for ( i = 0; i<16; i++ )
		{
			if ( input[i] == 0 )
				break;
			else
				input[i] = tolower( input[i] );
		}
		if ( strlen( input ) > 1 )
			ParseName( input );
	}

	fclose( pFile );

	int total = 0;
	for ( int i = 0; i < 756; i++ )
	{
		total = 0;
		for ( int j = 0; j<27; j++ )
			total += freqTable[i][j];
		freqTable[i][27] = total;
	}

	pFile = fopen( "doubledata.txt", "wt+" );

	for ( int i = 0; i<756; i++ )
	{
		for ( int j = 0; j < 28; j++ )
		{
			fprintf( pFile, "%d ", freqTable[i][j] );
		}
		fprintf( pFile, "\n" );
	}

	fclose( pFile );
}

char CDoubleMarkov::GetCharacterFromSet( char firstChar, char secondChar )
{
	int i = 0;
	int random = 0;
	int index = 0;

	if ( firstChar == 0 )
		index = 0;
	else if ( secondChar == 0 )
		index = 1 + CharacterIndex( firstChar );
	else
		index = GetIndex( firstChar, secondChar );

	if ( freqTable[index][27] != 0 )
	{
		random = rand()%freqTable[index][27];
		for ( i = 0; i<27; i++ )
		{
			random -= freqTable[index][i];
			if ( random <= 0 )
				break;
		}

		return IndexCharacter( i );
	}

	return '\'';
}

void CDoubleMarkov::GenerateName(int length, char *name)
{
	strcpy( name, "" );
	bool accepted = false;
	while ( !accepted )
	{
		name[0] = GetCharacterFromSet( 0, 0 );
		name[1] = GetCharacterFromSet( name[0], 0 );

		int l = 2;
		while ( l <= length )
		{
			name[l] = GetCharacterFromSet( name[l - 2], name[l - 1] );
			l++;
		}
		name[l] = 0;

		accepted = IsAcceptable(name);
	}
}
