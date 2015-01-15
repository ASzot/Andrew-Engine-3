#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Graphics.h"
#include <fstream>
#include <istream>
#include <algorithm>
#include "PhysicsManager.h"
#include "EventManager.h"
#include "PlayerManager.h"


class CScene : public CDataObject
{
public:
	CScene();
	virtual ~CScene();

	void Render(ID3D11DeviceContext* context,CameraInfo& cam);

	//To update things such as the lighting and what not
	void Update(GameStates gameState,double time, float elapsedTime,CameraInfo& cam,ID3D11Device* device,ID3D11DeviceContext* context);

	bool Init(CEventManager* eventManager,HINSTANCE instance,HWND mainWindow);

	bool InitResources(ID3D11Device* device,const DXGI_SURFACE_DESC* sd,Camera* cam,CEventManager* eventManager);

	void Shutdown();
	void DestroyResources();

	void Serilize(CDataArchiver* dataArchiver);

	void OnLeftMouseBtnDown( WPARAM mouseState, int x, int y, Camera& cam );

private:
	CGraphics* mGraphics;
	CGameObjMgr* m_pObjMgr;
	CPhysicsManager* m_pPhysicsMgr;
	CEventSnooper mListener;
	CPlayerManager* m_PlayerManager;
	// Set to false to quit the loading screen.
	bool* m_QuitLoadingScreen;
	bool m_bFirstLoad;
	CMainPlayer m_MainPlayer;

private:
	XMFLOAT3 StrafeCamera(float d,XMFLOAT4 right,XMFLOAT3 pos);
	XMFLOAT3 WalkCamera(float d,XMFLOAT3 look,XMFLOAT3 pos);

public:
	// Loads a new scene completely destroys the old one
	bool LoadScene(const char* filename, ID3D11Device* device);
	void OnResize(const DXGI_SURFACE_DESC* sd, ID3D11Device* device);
private:
	void CheckEvents(CameraInfo& cam);
};