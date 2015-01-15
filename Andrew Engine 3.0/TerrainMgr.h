//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once
#include "manager.h"
#include "Terrain.h"

class CTerrainMgr :
	public CManager
{
public:
	CTerrainMgr(HINSTANCE hInstance, HWND hwnd);
	virtual ~CTerrainMgr(void);

	virtual bool Init(CEventManager* eventManager,CHashedString messageForWindowCreation);
	virtual bool InitializeResources(ID3D11Device* device);
	virtual void Shutdown(void);
	virtual void DestroyResources(void);
	virtual void Update(float dt);
	virtual void Serilize(CDataArchiver* archiver);
	
	bool LoadScene(InitInfo ii);
	void DestroyGameAssets();
	
	void Render( CEffectManager* effectManager, CameraInfo& cam, ExtraRenderingData& erd );
	void RenderToShadowMap( CEffectManager* effectManager, CameraInfo& cam );

protected:
	virtual BOOL CreateDlgWindow();
	virtual void OnDialogQuit();

private:
	CTerrain* m_pTerrain;
};

