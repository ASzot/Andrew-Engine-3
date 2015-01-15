//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once 

#include "WindowManager.h"
#include "Camera.h"
#include <sstream>
#include "DXUTgui.h"
#include "WindowCreatorFoundation.h"


#ifndef FATAL_ERROR
#define FATAL_ERROR(x) { MessageBoxA(0,x,"Fatal Error",MB_OK);  }
#endif

// Sadly most of this file is depricated a few useful things remain such as the text renderer.
// The reason being because it was one of the first things I had written.
//TODO:
// Make this whole system thing better and improve how it interacts with the rest of the engine.


class CSystem
{
private:
	struct TempRenderText
	{
		TempRenderText()
		{
			TimeAlive = TIME_TEXT_ALIVE;
			CurTime = 0.0f;
		}

		void Update( double dt )
		{
			CurTime += dt;
		}

		bool IsDead()
		{
			if ( TimeAlive < CurTime )
				return true;
			return false;
		}
		double CurTime;
		double TimeAlive;
		std::wstring Message;
	};

public:
	CSystem(HINSTANCE instance, HWND hMainWnd);
	~CSystem();

	void CreateMessageBox(LPCWSTR text);

	inline void ToggleFullscreen();

	// This returns a 'handle' to the exit variable for the loading screen.
	// Just set the variable to false and the window disapears its magic.
	bool* CreateLoadingScreen(int numberOfSteps);
	void StepLoadingDlg();

	bool Initialize(CEventManager* eventManager);

	bool InitResources(ID3D11Device* device);

	GameStates Update(GameStates gameState, float dt);
	void Render(GameStates gameState,Camera cam);
	HRESULT OnResizedSwapChain(ID3D11Device* device, IDXGISwapChain* swapChain, const DXGI_SURFACE_DESC* backBufferSurfaceDesc);

	void Shutdown();
	void DestroyResources();
	void OnReleaseSwapChain();
	
	char* GetSaveFilename(bool& succeeded,std::string& levelName);
	char* GetOpenFilename(bool& succeeded);

	void SendTextForRendering( std::wstring text );

	void DisplayConsoleText( std::wstring message );

	//All of these are called through the event managers registered system
	void OnLoad();
	void OnQuit();
	void OnFinishedLoading();
	void OnEnterBuilderMode();
	void OnExitBuilderMode();

private:
	//No reason to make public it isnt going to be accessed by event manager
	GameStates OnPaused();

	void DrawPhysObjsNotUpdTxt(int textYPos);

private:
	HINSTANCE mhInstance;
	HWND mhMainWnd;
	HWND mToolboxWnd;
	CEventSnooper mEventListener;
	bool mIsPaused;
	CDXUTDialogResourceManager mDialogResourceManager;
	CDXUTTextHelper* mTextHelper;
	CWindowManager* mWindowManager;
	bool* m_bhLoadingScreenExit;
	WCF::LoadingBox* m_dLoadingBox;
	D3DXCOLOR m_DefaultTextColor;
	D3DXCOLOR m_WarningTextColor;
	std::vector< std::wstring > m_TextForRendering;
	typedef std::vector< TempRenderText > TempTextList;
	TempTextList m_TempRenderText;
};