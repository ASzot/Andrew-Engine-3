#ifndef CAPP
#define CAPP

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Input.h"
#include "GameLogic.h"
#include "System.h"
#include "WaitDlg.h"

class CApp
{
public:
	CApp(bool windowed);
	~CApp();

	HRESULT OnCreateResources(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext);

	void OnDestroyDevice(void* pUserContext);

	void OnFrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                  float fElapsedTime, void* pUserContext);

	HRESULT OnResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
	
	bool Initialize(HINSTANCE hInstance, HWND hMainWnd);

	void OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );

	void OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                       bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                       int xPos, int yPos, void* pUserContext);

	LRESULT MessageProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext );

	void DestroyApp();

	void Update(double fTime, float fElapsedTime, void* pUserContext);

	void OnReleasingSwapChaing(void* pUserContext);



private:
	CInput* mInput;
	GameStates mGameState;
	CGameLogic mGameLogic;
	CSystem* mSystem;
	CEventManager* mEventManager;
	bool m_bWindowed;
	std::string m_NameOfLevelLoaded;
public:
	bool IsWindowed(void);
};

#endif