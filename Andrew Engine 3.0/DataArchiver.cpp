//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "DataArchiver.h"
#include "StringHelper.h"

CDataArchiver::CDataArchiver(void)
{

}


CDataArchiver::~CDataArchiver(void)
{
}

bool CDataArchiver::OpenForSave(const char* filename)
{
	std::string folderName("data/Levels/");
	std::string extension( ".txt" );
	std::string name( filename );
	std::string finalFile = ( folderName + name + extension ).c_str();
	mOutstream.open( finalFile,ios::trunc );
	if(!mOutstream)
	{
		int messageBoxId = MessageBoxA(0,"The file you have tried to save to already exists.\nDo you want to overwrite it?","Confirm Save",MB_ICONEXCLAMATION | MB_YESNO);
		if(messageBoxId != IDYES)
			return false;
	}
	mOutstream.clear();
	return true;
}

void CDataArchiver::CloseOutputDataStream()
{
	mOutstream.close();
}
