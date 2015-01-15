//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "StringHelper.h"


StringHelper::StringHelper(void)
{
}


StringHelper::~StringHelper(void)
{
}

bool StringHelper::IsLetter( char character )
{
	if ( 'A' < character && character < 'Z' )
		return true;
	else if ( 'a' < character && character < 'z' )
		return true;
	return false;
}
