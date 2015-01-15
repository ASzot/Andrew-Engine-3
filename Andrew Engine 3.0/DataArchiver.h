#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <Windows.h>
#include <xnamath.h>

using namespace std;

class CDataArchiver
{
private:
	ofstream mOutstream;
	bool mIsSaving;

public:
	CDataArchiver(void);
	~CDataArchiver(void);

	bool OpenForSave(const char* filename);

	void CloseOutputDataStream();

	bool IsSaving() { return mIsSaving; }

	void WriteToStream(int data)			{ mOutstream<<data; }
	void WriteToStream(float data)			{ mOutstream<<data; }
	void WriteToStream(double data)			{ mOutstream<<data; }
	void WriteToStream(char* data)			{ mOutstream<<data; }
	void WriteToStream(char data)			{ mOutstream<<data; }
	void WriteToStream(const char* data)	{ mOutstream<<data; }
	void WriteToStream(std::string& data)   { mOutstream<<data; }
	void WriteToStream(std::wstring& data)  { mOutstream<<data.c_str(); }
	void WriteToStream(LPCWSTR data)		{ mOutstream<<data; }
	void WriteToStream(bool data)			{ mOutstream<<(int)data; }
	void WriteToStream(XMFLOAT3 data)
	{
		WriteToStream( data.x );
		Space();
		WriteToStream( data.y );
		Space();
		WriteToStream( data.z );
	}
	void WriteToStream(UINT data)			{ mOutstream<<data; }
	void Space() { WriteToStream(' '); }

	void WriteFloat3ToStream(XMFLOAT4 data) { WriteToStream( XMFLOAT3( data.x, data.y, data.z ) ); }


};