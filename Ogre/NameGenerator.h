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
