//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "RenderEffects.h"
#include "ObjectManager.h"
#include "Graphics.h"


RenderEffects::RenderEffects(HINSTANCE hInstance,HWND mainWindow)
{
	m_pRefractionTexture = 0;
	m_pReflectionTexture = 0;
	m_bRenderReflections = false;
}


RenderEffects::~RenderEffects(void)
{
}


bool RenderEffects::InitResources(const DXGI_SURFACE_DESC* sd)
{
	m_pRefractionTexture = new CRenderTexture();
	assert( m_pRefractionTexture );

	int height	= sd->Height;
	int width	= sd->Width;

	if ( !m_pRefractionTexture->Initialize( height, width ) )
		return false;

	m_pReflectionTexture = new CRenderTexture();
	assert( m_pReflectionTexture );

	if ( !m_pReflectionTexture->Initialize( height, width ) )
		return false;

	return true;
}


bool RenderEffects::Init(CEventManager* eventManager)
{
	//assert(m_pBlurEffect);
	//m_pBlurEffect->Init(eventManager,CHashedString("c6CreateBlurWindow"));
	return true;
}


void RenderEffects::Shutdown(void)
{
	SafeDelete( m_pReflectionTexture );
	SafeDelete( m_pRefractionTexture );
}


void RenderEffects::DestroyResources(void)
{
	ReleaseResources( m_pReflectionTexture );
	ReleaseResources( m_pRefractionTexture );
}

void RenderEffects::OnResize(UINT width,UINT height)
{
	m_pReflectionTexture->Resize( width, height );
	m_pRefractionTexture->Resize( width, height );
}

bool RenderEffects::HasTwoRenderPasses()
{
	return false;
}


void RenderEffects::Render(CEffectManager* effectManager)
{
	/*m_pBlurEffect->Render(DXUTGetD3D11DeviceContext(),effectManager);*/
}


void RenderEffects::Prerender( CGraphics* pGraphics, CObjectManager* objMgr, CWaterManager* pWaterManager, CEffectManager* effectManager, CameraInfo& ci )
{
	if ( m_bRenderReflections )
	{
		this->RenderReflectionToTexture( effectManager, ci );
		// Do a regular scene render. 
		pGraphics->RenderScene( DXUTGetD3D11DeviceContext(), ci, RenderPass::RenderReflectionPass );
		this->ResetBackBufferRenderTarget();
	}

	ID3D11ShaderResourceView* refractReflectTextures[2] = 
	{ 
		m_pRefractionTexture->GetShaderResourceView(), 
		m_pReflectionTexture->GetShaderResourceView() 
	};
	
	// Entirely takes care of rendering any refractable objects.
	pWaterManager->RenderWaterObjects( effectManager, ci, refractReflectTextures, this );
}


void RenderEffects::AfterPrerender(void)
{
	/*ID3D11RenderTargetView* renderTargets[1] = { DXUTGetD3D11RenderTargetView() };
	DXUTGetD3D11DeviceContext()->OMSetRenderTargets( 1, renderTargets, DXUTGetD3D11DepthStencilView() );*/
}


void RenderEffects::AfterRender( CEffectManager* effectManager )
{
	/*float colors[4] = { 0.0f,0.0f,0.0f,0.0f };
	DXUTGetD3D11DeviceContext()->ClearRenderTargetView( DXUTGetD3D11RenderTargetView(), colors );
	DXUTGetD3D11DeviceContext()->ClearDepthStencilView( DXUTGetD3D11DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
	
	m_pBlurEffect->DrawScreenQuad(DXUTGetD3D11DeviceContext(),effectManager);*/
}

void RenderEffects::RenderRefractionToTexture(  CEffectManager* effectManager, CameraInfo& ci, float waterHeight )
{
	m_pRefractionTexture->SetRenderTarget();
	m_pRefractionTexture->ClearRenderTarget( 0.35f, 0.35f, 0.35f, 1.0f );

	effectManager->mRefractFX->SetView( ci.View );
	effectManager->mRefractFX->SetProj( ci.Proj );

	XMFLOAT4 clipPlane = XMFLOAT4( 0.0f, -1.0f, 0.0f, waterHeight + 0.1f );
	effectManager->mRefractFX->SetClipPlane( clipPlane );
	effectManager->mTerrainFX->SetClipPlane( clipPlane );
}


void RenderEffects::RenderReflectionToTexture( CEffectManager* effectManager, CameraInfo& ci )
{
	m_pReflectionTexture->SetRenderTarget();	
	m_pReflectionTexture->ClearRenderTarget( 0.0f, 0.0f, 0.0f, 1.0f );
}

void RenderEffects::ResetBackBufferRenderTarget()
{
	ID3D11RenderTargetView* rtv = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* dsv = DXUTGetD3D11DepthStencilView();

	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();

	DXUTGetD3D11DeviceContext()->OMSetRenderTargets( 1, &rtv, dsv );
	context->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f );
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	context->ClearRenderTargetView( rtv, color );
}

ID3D11ShaderResourceView* RenderEffects::GetRefractionTexture()
{
	return m_pRefractionTexture->GetShaderResourceView();
}