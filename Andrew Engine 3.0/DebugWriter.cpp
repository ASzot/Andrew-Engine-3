#include "DebugWriter.h"
#include "DXUT.h"

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================


std::ofstream DebugWriter::m_Outstream = std::ofstream();

void DebugWriter::WriteToDebugFile( const char* message,const char* funcOfFailure )
{
	m_Outstream << message;
	m_Outstream << " :: Failed At:";
	m_Outstream << funcOfFailure;
	m_Outstream.put( '\n' );
}

void DebugWriter::WriteToDebugFile( const wchar_t* message, bool immediateBreak, const char* file, const unsigned int line )
{
	DXUTTrace( file, (DWORD)line, E_FAIL, message, immediateBreak);

	m_Outstream << message;
	m_Outstream << "\n";
	m_Outstream << "Failed at: ";
	m_Outstream << "Line ";
	m_Outstream << line;
	m_Outstream << ", In File ";
	m_Outstream << file;
	m_Outstream.put( '\n' );
	m_Outstream.put( '\n' );
}

void DebugWriter::CheckValue( HRESULT hr, const char* file /*= __FILE__*/, const unsigned int line /*= __LINE__ */ )
{
	if ( hr == S_OK )
		return;

	DXUTTrace( file, (DWORD)line, hr, L"Failed!", true);


	m_Outstream << "Failed at: ";
	m_Outstream << "Line ";
	m_Outstream << line;
	m_Outstream << ", In File ";
	m_Outstream << file;
	m_Outstream.put( '\n' );
	m_Outstream.put( '\n' );
}
