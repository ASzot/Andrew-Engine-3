#include "App.h"

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================


CApp::CApp(bool windowed)
	: mGameState(eRunning)
{
	m_bWindowed = windowed;
	m_NameOfLevelLoaded = "";
}

CApp::~CApp()
{
}

HRESULT CApp::OnCreateResources(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext)
{
	CWaitDlg dlg;
	dlg.ShowDialog( L"Initializing the resources" );

	if(!mSystem->InitResources(pd3dDevice))
		return E_FAIL;
	const char* defLevel = "Default";
	// mGameLogic->LoadScene( defLevel, pd3dDevice );
	m_NameOfLevelLoaded = std::string( defLevel );
	//Create all resources that need the device to initialize

	bool result = mGameLogic.InitResources(pd3dDevice,pBackBufferSurfaceDesc,mEventManager);

	if(!result)
	{
		MessageBoxA(NULL,"Couldnt create the resources!","Fatal Error",MB_OK);
		return E_FAIL;
	}

	//mGameLogic.LoadScene( "data/Levels/Default.txt", pd3dDevice );

	dlg.DestroyDialog();

	return S_OK;
}

void CApp::OnDestroyDevice(void* pUserContext)
{
	//Destroy all resources that were created with the device
	mSystem->DestroyResources();
	mGameLogic.DestroyResources();
	//DestroyApp();
}

void CApp::OnFrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                  float fElapsedTime, void* pUserContext)
{
	mGameLogic.Render(mEventManager->GetGameState());
	mSystem->Render(mGameState, mGameLogic.GetCamera( ) );
}

HRESULT CApp::OnResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	mSystem->OnResizedSwapChain(pd3dDevice,pSwapChain,pBackBufferSurfaceDesc);
	mGameLogic.OnResize(pBackBufferSurfaceDesc,pd3dDevice);
	return S_OK;
}

bool CApp::Initialize(HINSTANCE hInstance, HWND hMainWnd)
{
	CWaitDlg initDialog;
	initDialog.ShowDialog( L"Initializing..." );
	

	mInput = new CInput;
	assert(mInput);

	mSystem = new CSystem(hInstance,hMainWnd);
	assert(mSystem);

	mEventManager = new CEventManager;
	assert(mEventManager);

	mSystem->Initialize(mEventManager);

	mEventManager->RegisterSystem(mSystem);


	if(!mGameLogic.Initialize(mEventManager,hInstance,hMainWnd))
		return false;

	initDialog.DestroyDialog();

	return true;
}

void CApp::OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	//All input from the keyboard is handled here. The events are then sent to the event handler

	//Get the event from the input
	CHashedString triggeredEvent = mInput->ProcessInput(nChar,bKeyDown,bAltDown,mGameState);

	if ( triggeredEvent == CHashedString("c1ResetLevel"))
	{
		CWaitDlg dlg;
		dlg.ShowDialog(L"Reloading the level...");
		mGameLogic.LoadScene( "data/Levels/Default.txt", DXUTGetD3D11Device() );
		dlg.DestroyDialog();
		return;
	}
	
	// Check for any control event types.
	if ( triggeredEvent.GetEventType() == eControlEventType )
		mEventManager->SendDirectMessage( triggeredEvent );

	if ( triggeredEvent.GetEventType() == eNothingEventType )
		return;

	mEventManager->SendMessageToListener( eInputEventType,	triggeredEvent );
	mEventManager->SendMessageToListener( eSystemEventType, triggeredEvent );
	mEventManager->SendMessageToListener( eParticleSystem,	triggeredEvent );
	mEventManager->SendMessageToListener( eEffectEventType, triggeredEvent );
	mEventManager->SendMessageToListener( eAnimEventType,	triggeredEvent );
	mEventManager->SendMessageToListener( eTerrainEventType,triggeredEvent );
}

void CApp::OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                       bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                       int xPos, int yPos, void* pUserContext)
{
	
}

LRESULT CApp::MessageProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		mGameLogic.OnLeftMouseDown(wParam,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		mGameLogic.OnMouseDown(wParam,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		mGameLogic.OnMouseUp(wParam,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		mGameLogic.OnMouseMove(wParam,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		return 0;
	case WM_CLOSE:
		int x = 0;
	}

	mGameLogic.HandleMessages(hWnd,uMsg,wParam,lParam);
	return 0;
}

void CApp::DestroyApp()
{
	SafeDelete(mEventManager);
	DestroyObject(mSystem);
	SafeDelete(mInput);
	mGameLogic.Shutdown();
}

void CApp::Update(double fTime, float fElapsedTime, void* pUserContext)
{
	mGameState = mEventManager->GetGameState();
	if(mGameState == eSaving)
	{
		CDataArchiver* dataArchiver = new CDataArchiver();
		assert(dataArchiver);
		bool suceeded = false;
		std::string levelName;
		const char* desiredFile;
		if ( m_NameOfLevelLoaded == "" )
		{
			desiredFile = mSystem->GetSaveFilename(suceeded,levelName);
		}
		else
		{
			// This means we have a named level already open.
			desiredFile = m_NameOfLevelLoaded.c_str();
			suceeded = true;
		}
		
		if(suceeded)
		{
			if(dataArchiver->OpenForSave(desiredFile))
			{
				//Just write the level name here
				dataArchiver->WriteToStream('n');
				dataArchiver->WriteToStream(' ');
				dataArchiver->WriteToStream("MyLevel");
				dataArchiver->WriteToStream('\n');
				dataArchiver->WriteToStream('\n');
				mGameLogic.Serilize(dataArchiver);
			}
			dataArchiver->CloseOutputDataStream();
		}
		mGameState = eRunning;
		SafeDelete(dataArchiver);
	}
	else if(mGameState == eLoadingLevel)
	{
		bool succeeded = false;
		char* desiredFile = mSystem->GetOpenFilename(succeeded);
		if(succeeded)
		{
			mGameLogic.LoadScene(desiredFile,DXUTGetD3D11Device());
			m_NameOfLevelLoaded = desiredFile;
		}
		mGameState = eRunning;
	}

	mEventManager->SetGameState( mSystem->Update( mGameState, fElapsedTime ) );
	mGameLogic.Frame( fTime, fElapsedTime, mGameState, DXUTGetD3D11Device(), DXUTGetD3D11DeviceContext() );
}

void CApp::OnReleasingSwapChaing(void* pUserContext)
{
	mSystem->OnReleaseSwapChain();
}

bool CApp::IsWindowed(void)
{
	return m_bWindowed;
}
