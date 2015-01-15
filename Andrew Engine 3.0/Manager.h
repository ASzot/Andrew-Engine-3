#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "dataobject.h"
#include "EventManager.h"
#include "WindowCreatorFoundation.h"

class CEventManager;
class CEventSnooper;


// An interface for manager classes. Manages messages and creation of the editor dlg, also standardizes functions.
class CManager :
	public CDataObject
{
public:
	CManager(HINSTANCE hInstance, HWND hwnd,gEEventTypes eventType,WCF::ModalessWindow* window);
	virtual ~CManager(void);

public:
	virtual bool Init(CEventManager* eventManager,CHashedString messageForWindowCreation);
	virtual bool InitializeResources(ID3D11Device* device);
	virtual void Shutdown(void);
	virtual void DestroyResources(void);
	virtual void Update(float dt);
	virtual void Serilize(CDataArchiver* archiver) = 0;
	virtual void ListenToMessages(char* message);
	virtual void ProcessMessage(CHashedString message);

protected:
	WCF::ModalessWindow* mWindow;
	HINSTANCE mhInstance;
	HWND mhWnd;
	CEventSnooper mEventListener;
	CHashedString mToCreateWindow;

protected:
	virtual BOOL CreateDlgWindow() = 0;
	virtual void OnDialogQuit() = 0;
};

