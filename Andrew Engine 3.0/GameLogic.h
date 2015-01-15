#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "EventManager.h"
#include "Scene.h"
#include "DXUTsettingsdlg.h"
#include "ThirdPersonCamera.h"
#include "SoundManager.h"


class CGameLogic
{
public:

	bool Initialize(CEventManager* eventManager,HINSTANCE instance,HWND mainWindow);

	bool InitResources(ID3D11Device* device,const DXGI_SURFACE_DESC* sd,CEventManager* eventManager);

	void Frame(double fTime, float fElapsedTime,GameStates gameState,ID3D11Device* device,ID3D11DeviceContext* context);

	virtual void Shutdown();
	void DestroyResources();

	void Serilize(CDataArchiver* dataArch);

	Camera GetCamera()
	{
		return mCam;
	}

	void Render(GameStates gameState);

	void OnResize(const DXGI_SURFACE_DESC* sd,ID3D11Device* device);

	void HandleMessages(HWND hwnd,UINT uMsg,WPARAM wParam, LPARAM lParam);

	void OnMouseDown(WPARAM mouseState, int x, int y);
	void OnMouseUp(WPARAM mouseState,int x, int y);
	void OnMouseMove(WPARAM mouseState,int x, int y);

	void OnLeftMouseDown(WPARAM mouseState,int x,int y);

private:
	CScene* mScene;
	Camera mCam;
	POINT mLastMousePos;
	CThirdPersonCamera mTHCam;
	CProcessManager* m_pProcessManager;
	CSoundManager* m_pSoundManager;
	double m_dTimeLastFrame;
	bool m_bFirstLoad;
public:
	void Unload();
	bool LoadScene(const char* filename, ID3D11Device* device);
};
