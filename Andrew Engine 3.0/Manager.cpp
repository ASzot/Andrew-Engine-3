//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "Manager.h"


CManager::CManager(HINSTANCE hInstance, HWND hwnd,gEEventTypes eventType,WCF::ModalessWindow* window)
	: mEventListener(eventType)
{
	mWindow = window;
	mhInstance = hInstance;
	mhWnd = hwnd;
}


CManager::~CManager(void)
{
}


bool CManager::Init(CEventManager* eventManager,CHashedString message)
{
	eventManager->RegisterListener(&mEventListener);
	mToCreateWindow = message;

	return true;
}


bool CManager::InitializeResources(ID3D11Device* device)
{
	return true;
}


void CManager::Shutdown(void)
{
	if(mWindow)
	{
		delete mWindow;
		mWindow = 0;
	}
}


void CManager::DestroyResources(void)
{
}


void CManager::Update(float dt)
{
	CHashedString message;
	while(mEventListener.HasPendingMessages())
	{
		message = mEventListener.GetRecievedMessage();
		if(message == mToCreateWindow && !mWindow->mIsWindow)
			CreateDlgWindow();
		ProcessMessage( message );
	}
	if(mWindow->WasExitButtonPressed())
	{
		OnDialogQuit();
	}
	if(!mWindow->mIsWindow)
	{
		mWindow->DestroyWindow();
	}
	int i = 0;
	std::vector<char*>::iterator iter = mWindow->mMessages.begin();
	while(!mWindow->mMessages.empty())
	{
		ListenToMessages(mWindow->mMessages.at(i));
		
		mWindow->mMessages.erase(iter,iter + 1);
		++i;
	}
}

void CManager::ProcessMessage(CHashedString message)
{

}


void CManager::ListenToMessages(char* message)
{
	UNREFERENCED_PARAMETER(message);
}