#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "BlurEffect.h"
#include "RenderTextureClass.h"
#include "WaterManager.h"

// Pre-defines.
class CObjectManager;
class CGraphics;

class RenderEffects
{
public:
	RenderEffects(HINSTANCE hInstance,HWND mainWindow);
	~RenderEffects(void);

private:
	void RenderReflectionToTexture( CEffectManager* effectManager, CameraInfo& ci );
	
private:
	CRenderTexture* m_pRefractionTexture;
	CRenderTexture* m_pReflectionTexture;
	bool m_bRenderReflections;
	
public:
	bool InitResources(const DXGI_SURFACE_DESC* sd);
	bool Init(CEventManager* eventManager);

	void RenderRefractionToTexture( CEffectManager* effectManager, CameraInfo& ci, float waterHeight );
	void ResetBackBufferRenderTarget();

	void Shutdown(void);
	void DestroyResources(void);

	ID3D11ShaderResourceView* GetRefractionTexture();

	void OnResize(UINT width,UINT height);

	bool HasTwoRenderPasses();

	void Render(CEffectManager* effectManager);
	void Prerender( CGraphics* pGraphics, CObjectManager* objMgr, CWaterManager* pWaterManager, CEffectManager* effectManager, CameraInfo& ci );
	void AfterPrerender(void);
	void AfterRender(CEffectManager* effectManager);
};

