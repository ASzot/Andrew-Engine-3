#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "HashedString.h"


#ifndef SafeDelete
#define SafeDelete(x) { if(x) { delete x; x = 0; } }
#endif

using namespace std;

////////////////////////////
///GLOBAL DECLARATIONS/////
//////////////////////////



enum EventType { eCreateObject, eDestroyObject, ePause, eResume, eQuit, eNothing, eToggleFullscreen };

enum GameStates { eRunning, ePaused, eQuiting, eLoading, eBuildingLevel, eSaving, eLoadingLevel }; 

class IListener
{
public:
	IListener(gEEventTypes listenerType)
		: mListenerType(listenerType)
	{
		
	}
	virtual ~IListener()
	{}

	gEEventTypes GetListenerType() { return mListenerType; }

	virtual void SendMessageToListener(gEEventTypes listenerType,CHashedString eventType) = 0;

	virtual void RecieveMessage(CHashedString eventType) = 0;
	virtual CHashedString GetRecievedMessage() = 0;
	bool HasPendingMessages() 
	{
		if(mRecievingMessages.size())
			return true;
		return false;
	}
	virtual void RemoveAllOfMessage(CHashedString hs)
	{
		for(int i = 0; i < mRecievingMessages.size(); ++i)
		{
			if(mRecievingMessages.at(i) == hs)
			{
				std::vector<CHashedString>::iterator iter = mRecievingMessages.begin() + i;
				mRecievingMessages.erase(iter,iter + 1);
			}
		}
	}

protected:
	gEEventTypes mListenerType;
	std::vector<CHashedString> mRecievingMessages;
	int mMessageFlags;
};




