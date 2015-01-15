//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "System.h"

CSystem::CSystem(HINSTANCE instance, HWND hwnd)
	: mEventListener(eSystemEventType)
{
	mhInstance = instance;
	mhMainWnd = hwnd;
	m_bhLoadingScreenExit = 0;
	m_dLoadingBox = 0;
	m_DefaultTextColor = D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f );
	m_WarningTextColor = D3DXCOLOR( 1.0f, 0.0f, 0.0f, 1.0f );
}

CSystem::~CSystem()
{
	mTextHelper = NULL;
}

void CSystem::CreateMessageBox(LPCWSTR text)
{
	//Just create a generic message box
	MessageBox(mhMainWnd,text,L"Message Box",MB_OK);
}

inline void CSystem::ToggleFullscreen()
{
	// Ya this doesn't work for some reason.
	//DXUTToggleFullScreen();
}

bool CSystem::Initialize(CEventManager* eventManager)
{
	eventManager->RegisterListener(&mEventListener);
	mWindowManager = new CWindowManager();
	assert(mWindowManager);

	PhysicsSynced( true );

	return true;
}

bool CSystem::InitResources(ID3D11Device* device)
{
	HRESULT hr;
	V_RETURN(mDialogResourceManager.OnD3D11CreateDevice(device,DXUTGetD3D11DeviceContext()));
	mTextHelper = new CDXUTTextHelper(device,DXUTGetD3D11DeviceContext(),&mDialogResourceManager,15);

	// This actually does work. Just don't use the DXUTGetHWND()
	//TODO:
	// Incoperate this and make it work.


	//if(!mWindowManager->CreateMainWindowMenu(DXUTGetHWND()))
		//FATAL_ERROR("Couldn't create the main wnd menu");

	return true;
}

void CSystem::DestroyResources()
{
	SAFE_DELETE(mTextHelper);
	mDialogResourceManager.OnD3D11DestroyDevice();
}

void CSystem::Shutdown()
{
	SafeDelete(mWindowManager);
}

GameStates CSystem::Update(GameStates gameState, float dt)
{
	// Handle all game state events.
	CHashedString triggeredEvent;
	while(mEventListener.HasPendingMessages())
	{
		triggeredEvent = mEventListener.GetRecievedMessage();
		if(triggeredEvent == "c2GameStateResumed")
		{
			//Put any handler code here for when the game is resumed
		}
		if(triggeredEvent == "c2GameStatePaused")
		{
			mIsPaused = true;
			return OnPaused();
		}
		else if(triggeredEvent == "c2Saving")
		{
			return eSaving;
		}
		else if(triggeredEvent == "c2OpenUserPreferences")
		{
			if(mWindowManager->CreateModalDialogBox(IDD_DIALOG_USER_PREFERENCES,mhInstance,mhMainWnd) == IDC_BUTTON_UP_SAVE)
			{
				//TODO:
				//Put code here
			}
		}
	}

	// Process any recently clicked buttons. 
	// The only problem with this is click events can be swallowed up.
	UINT windowFlags = mWindowManager->ProccessWindows();
	if(windowFlags)
	{
		switch(windowFlags)
		{
		case IDC_BUTTON_RESUME:
			return eRunning;
			break;
		case IDC_BUTTON_PD_EXIT:
			return eQuiting;
			break;
		case IDC_BUTTON_PD_BUILD_LEVEL:
			return eBuildingLevel;
			break;
		case IDC_BUTTON_TB_ADD_OBJECT:
			if(mWindowManager->CreateModalDialogBox(IDD_DIALOG_ADD_OBJECT,DXUTGetHINSTANCE(),DXUTGetHWND()) == IDC_BUTTON_AO_ADD_OBJECT)
			{
				// To add a object by sending a message you must send the object added message and call the create object method order doesnt matter but the consequences
				// will be fatal if not followed correctly.
				mEventListener.CreateObject(mWindowManager->GetTextBuffer());
				mEventListener.SendMessageToListener(eInputEventType,CHashedString("c1ObjectAdded"));
			}
			break;
		case IDC_BUTTON_TB_DELETE_SELECTED_OBJECT:
			mEventListener.SendMessageToListener(eInputEventType,CHashedString("c1SelectedObjectDeleted"));
			break;
		case IDC_BUTTON_TB_ADD_CUSTOM_OBJECT:
			mEventListener.SendMessageToListener(eInputEventType,CHashedString("c1CreateAddObjectDlg"));
			break;
		}
	}

	if ( m_dLoadingBox )
	{
		// If we got here then the window should be up but just in case.
		if ( m_dLoadingBox->m_bIsWindow )
		{
			// The window will update itself.
			
			// If this 'handle' variable is set to false then the window is destroyed.
			if ( m_bhLoadingScreenExit )
			{
				m_dLoadingBox->QuitWindow();
				delete m_dLoadingBox;
				// Also delete the exit 'handle' we created earlier.
				delete m_bhLoadingScreenExit;
				m_dLoadingBox = 0;
				m_bhLoadingScreenExit = 0;
			}
		}
	}

	// Update the clock on our temp text.
	std::vector<TempTextList::iterator> elementsToErase;
	for ( TempTextList::iterator i = m_TempRenderText.begin(); i != m_TempRenderText.end(); ++i )
	{
		( *i ).Update( (double)dt );
		if ( ( *i ).IsDead() )
			elementsToErase.push_back( i );
	}

	for ( std::vector<TempTextList::iterator>::iterator i = elementsToErase.begin(); i != elementsToErase.end(); ++i )
		m_TempRenderText.erase( ( *i ) );

	return gameState;
}

void CSystem::OnLoad()
{
	//This is obviously called on any load we set the state to loading
}

void CSystem::OnQuit()
{
	//Put any shutdown code here
	PostQuitMessage(0);
}

void CSystem::OnFinishedLoading()
{

}

GameStates CSystem::OnPaused()
{
	mWindowManager->CreateDialogWindow(IDD_DIALOG_PAUSE,DXUTGetHINSTANCE(),DXUTGetHWND());
	return ePaused;
}

void CSystem::Render(GameStates gameState,Camera cam)
{
	//Render the text
	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR,L"FRAME STATS");
	mTextHelper->Begin();
	mTextHelper->SetInsertionPos(2,0);
	mTextHelper->SetForegroundColor(m_DefaultTextColor );
	mTextHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
	mTextHelper->DrawTextLine(DXUTGetDeviceStats());

	int textHeight = 60;
	int heightInc = 20;

#if defined( DEBUG) || defined ( _DEBUG)
	//ALL GAME  STATES MUST BE HANDLED HERE OR WE WILL GET A UNHANDLED EXCEPTION
		mTextHelper->SetInsertionPos(2,40);
		WCHAR* text;
		switch(gameState)
		{
		case eRunning:
			text = L"Running";
			break;
		case ePaused:
			text = L"Paused";
			break;
		case eLoading:
			text = L"Loading";
			break;
		case eBuildingLevel:
			text = L"Building a Level";
			break;
		case eSaving:
			text = L"Saving a Level";
			break;
		case eLoadingLevel:
			text = L"Loading a Level";
			break;
		}
		mTextHelper->DrawTextLine(text);
		if(mEventListener.GetGameState() == eBuildingLevel)
		{
			
			mTextHelper->SetInsertionPos( 2, textHeight );
			textHeight += heightInc;
			int selectedObject = mEventListener.GetSelectedObject();
			//This is so the character is the correct number (think about it)
			WCHAR t = ( '0' + selectedObject );
			mTextHelper->DrawTextLine( &t );
			mTextHelper->SetInsertionPos( 2, textHeight );
			textHeight += heightInc;

			t = ( '0' + mEventListener.GetSelectedLight() );
			mTextHelper->DrawTextLine( &t );
			t = ( '0' + mEventListener.GetSelectedWaterSystem() );
			mTextHelper->SetInsertionPos( 2, textHeight );
			textHeight += heightInc;

			mTextHelper->DrawTextLine( &t );

			WCHAR* mgrText;
			switch( mEventListener.GetSelectedMgr() )
			{
			case eManager::eLightMgr:
				mgrText = L"Light Manager";
				break;
			case eManager::eObjectMgr:
				mgrText = L"Object Manager";
				break;
			case eManager::eParticleMgr:
				mgrText = L"Particle Manager";
				break;
			}

			mTextHelper->SetInsertionPos( 2, textHeight );
			textHeight += heightInc;

			mTextHelper->DrawTextLine( mgrText );

			WCHAR* physicsState;
			g_GetEventManager()->IsBuilderPhysicsEnabled() ? physicsState = L"Builder Physics Enabled." : physicsState = L"Builder Physics Disabled.";
			mTextHelper->SetInsertionPos( 2, textHeight );
			textHeight += heightInc;
			mTextHelper->DrawTextLine( physicsState );

			for ( int i = 0; i < m_TextForRendering.size(); ++i )
			{
				mTextHelper->SetInsertionPos( 2, textHeight );
				textHeight += heightInc;
				mTextHelper->DrawTextLine( m_TextForRendering.at( i ).c_str() );
			}
		}

		// Use a buffer for easy writing of the float data for the camera.
		std::ostringstream buffer;
		XMFLOAT3 pos = cam.GetPosition();
		buffer << pos.x;
		buffer << ",";
		buffer << pos.y;
		buffer << ",";
		buffer << pos.z;
		mTextHelper->SetInsertionPos( 2, textHeight );
		textHeight += heightInc;
		// Sadly there is we have to convert there is no other option.
		std::string t = buffer.str();
		std::wstring wt = WCF::ConversionTools::StrToWStr( t );
		mTextHelper->DrawTextLine( wt.c_str() );

		if ( !IsPhysicsSynced() && gameState == eRunning )
		{
			DrawPhysObjsNotUpdTxt( textHeight );
			textHeight += heightInc;
		}

		// Temporary text at the bottom left of the screen.
		float height = (float)DXUTGetWindowHeight();
		const float spacingPerTextElement = 20.0f;
		float totalSpaceNeeded = spacingPerTextElement * m_TempRenderText.size();
		const float startingSpot = height - totalSpaceNeeded;

		mTextHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 0.2f, 0.2f, 1.0f ) );
		float currentPosition = startingSpot;
		for ( TempTextList::iterator i = m_TempRenderText.begin(); i != m_TempRenderText.end(); ++i )
		{
			mTextHelper->SetInsertionPos( 2, currentPosition );
			mTextHelper->DrawTextLine( ( *i).Message.c_str() );
			currentPosition += spacingPerTextElement;
		}

#endif
	mTextHelper->End();
	DXUT_EndPerfEvent();

	// Clear all of the text for rendering for this frame.
	m_TextForRendering.clear();
}

HRESULT CSystem::OnResizedSwapChain(ID3D11Device* device, IDXGISwapChain* swapChain, const DXGI_SURFACE_DESC* backBufferSurfaceDesc)
{
	HRESULT hr;
	V_RETURN( mDialogResourceManager.OnD3D11ResizedSwapChain(device,backBufferSurfaceDesc));
	return S_OK;
}

void CSystem::OnReleaseSwapChain()
{
	mDialogResourceManager.OnD3D11ReleasingSwapChain();
}

void CSystem::OnEnterBuilderMode()
{
	mToolboxWnd = mWindowManager->CreateToolbox(mhMainWnd,mhInstance);
	if(!mToolboxWnd)
		FATAL_ERROR("Couldn't create the toolbox window!");
}

void CSystem::OnExitBuilderMode()
{
	mWindowManager->DestroyCurrentWindow(mToolboxWnd);
}

char* CSystem::GetSaveFilename(bool& s,std::string& levelName)
{
	char textBuffer[MAX_TEXT_BUFFER_SIZE];
	if(mWindowManager->CreateModalDialogBox(IDD_DIALOG_SAVE,DXUTGetHINSTANCE(),DXUTGetHWND()) == IDC_BUTTON_SD_ENTER)
	{
		levelName = mWindowManager->GetTextBuffer(INDEX_LEVEL_NAME_LOCATION);
		s = true;
		return mWindowManager->GetTextBuffer(INDEX_FILE_NAME_LOCATION);
	}
	
	s = false;
	return 0;
}

char* CSystem::GetOpenFilename(bool& s)
{
	char textBuffer[MAX_TEXT_BUFFER_SIZE];
	if(mWindowManager->CreateModalDialogBox(IDD_OPEN_DIALOG,DXUTGetHINSTANCE(),DXUTGetHWND()) == IDC_BUTTON_OD_OK)
	{
		s = true;
		return mWindowManager->GetTextBuffer(INDEX_FILE_NAME_LOCATION);
	}
	s = false;
	return 0;
}


bool* CSystem::CreateLoadingScreen(int numberOfSteps)
{
	if(m_bhLoadingScreenExit)
	{
		FATAL_ERROR("Tried to create two loading screens\nbool* CSystem::CreateLoadingScreen()");
		return 0;
	}
	
	m_bhLoadingScreenExit = new bool(false);

	m_dLoadingBox = new WCF::LoadingBox();
	if ( m_dLoadingBox->RunWindow( mhInstance, mhMainWnd, numberOfSteps) == FALSE )
		FATAL_ERROR("Couldn't create the window!\nbool* CSystem::CreateLoadingScreen()");

	// Return the handle
	return m_bhLoadingScreenExit;
}


void CSystem::StepLoadingDlg()
{
	assert( m_dLoadingBox );	
	if ( m_dLoadingBox->m_bIsWindow )
		m_dLoadingBox->StepDialog();
	else
		FATAL_ERROR("Tried to step the loading dialog, when there was no loading dialog!\nvoid CSystem::StepLoadingDlg()");
}

void CSystem::DrawPhysObjsNotUpdTxt(int textYPos)
{
	mTextHelper->SetForegroundColor( m_WarningTextColor );
	mTextHelper->SetInsertionPos( 2, textYPos );

	mTextHelper->DrawTextLine( L"The phys objects and the rendering objects are out of sync! Rebuild phys objects for phys simulation. ( B Mode: Alt + 1 )" );

	// Set the text color back to normal.
	mTextHelper->SetForegroundColor( m_DefaultTextColor );
}

void CSystem::SendTextForRendering( std::wstring text )
{
	m_TextForRendering.push_back( text );
}

void CSystem::DisplayConsoleText( std::wstring message )
{
	TempRenderText trt;
	trt.Message = message;

	m_TempRenderText.push_back( trt );
}