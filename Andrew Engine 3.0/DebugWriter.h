#pragma once
#include <fstream>
#include <iostream>
#include "DXUT.h"

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

// This class CAN NOT be used anywhere in the call hierarchy before CScene.
class DebugWriter
{
public:
	DebugWriter()
	{
	}
	~DebugWriter()
	{
		
	}

	static bool InitStream()
	{
		m_Outstream.open( "data/Debug/DebugLog.txt" );
		if ( m_Outstream )
			return true;
		return false;
	}

	static void CloseStream()
	{
		m_Outstream.close();
	}

	static void WriteToDebugFile( const wchar_t* message, bool immediateBreak = true, const char* file = __FILE__, const unsigned int line = __LINE__ );

	static void CheckValue( HRESULT hr, const char* file = __FILE__, const unsigned int line = __LINE__ );

	static void WriteToDebugFile(const char* message,const char* funcOfFailure);

private:
	static std::ofstream m_Outstream;
};


