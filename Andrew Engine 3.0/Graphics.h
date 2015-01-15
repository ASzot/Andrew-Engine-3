#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "ObjectManager.h"
#include "LightManager.h"
#include "ParticleManager.h"
#include "RenderEffects.h"
#include "AnimObjectMgr.h"
#include "ShadowMapper.h"
#include "Sky.h"
#include "TerrainMgr.h"


class CGraphics : public CManager
{
public:
	CGraphics(HINSTANCE hInstance, HWND hwnd);
	virtual ~CGraphics();

	bool InitResources(ID3D11Device* device,const DXGI_SURFACE_DESC* sd);
	bool Init(CEventManager* eventManager,HINSTANCE instance,HWND mainWindow,CGameObjMgr* ctrlMgr);

	bool LoadScene(const char* filename, ID3D11Device* device);

	//We will also need to pass down a camera here as well
	void Render(ID3D11DeviceContext* context,CameraInfo& cam);
	void RenderScene( ID3D11DeviceContext* context, CameraInfo& cam, RenderPass rp );

	void Shutdown();
	void DestroyResources();

	void Update(GameStates gameState,double time, float elapsedTime,CameraInfo& cam,ID3D11Device* device,ID3D11DeviceContext* context);

	void Serilize(CDataArchiver* dataArchiver);

private:
	void RenderShadowMap(ID3D11DeviceContext* context);
	void SetDlgInfo( GraphicsInfo info );

	// Unloads all game assets which were specific to that level.
	void Unload();
	
protected:
	virtual BOOL CreateDlgWindow();
	virtual void OnDialogQuit();

private:
	CWaterManager* m_pWaterMgr;
	CObjectManager* mObjectManager;
	CLightManager* mLightManager;
	CEffectManager* mEffectManager;
	CParticleManager* mParticleManager;
	RenderEffects* m_pRenderEffects;
	CAnimObjectMgr* m_pAnimObjMgr;
	CShadowMapper* m_pShadowMapper;
	CSky* m_pSky;
	CTerrainMgr* m_pTerrainMgr;

	ScenePerFrameInfo m_PerFrameInfo;
	bool m_bUseSkymap;
	bool m_bFlashSel;
	bool m_bFirstLoad;

public:
	void OnResize(const DXGI_SURFACE_DESC* sd, ID3D11Device* device);
};