#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "IObject.h"

enum BlendState { DefaultBS, TransparencyBS };
enum RasterizerState { DefaultRS, NoCullRS };
enum DepthStencilState { DefaultDSS };

class SRenderSettings
{

private:
	static ID3D11DepthStencilState*		mDefaultDSS;
	static ID3D11BlendState*			mDefaultBS;
	static ID3D11BlendState*			mTransparencyBS;
	static ID3D11RasterizerState*		m_pNoCullRS;

	static BlendState					m_BlendState;
	static RasterizerState				m_RasterizerState;
	static DepthStencilState			m_DepthStencilState;


public:

	static bool InitAllSettings(ID3D11Device* device);
	static void DestroyAllSettings();

	static void SetDefaultDSS()
	{
		DXUTGetD3D11DeviceContext()->OMSetDepthStencilState(mDefaultDSS,0.0f);
	}

	static void SetDefaultBS()
	{
		if ( m_BlendState != BlendState::DefaultBS )
		{
			float blendFactor[4]={0.0f,0.0f,0.0f,0.0f};
			DXUTGetD3D11DeviceContext()->OMSetBlendState(mDefaultBS,blendFactor,0xffffffff);
			m_BlendState = BlendState::DefaultBS;
		}
	}
	
	static void SetTransparencyBlending( float color )
	{
		if ( m_BlendState != BlendState::TransparencyBS )
		{
			float blendFactor[4] = { color, color, color, color};
			DXUTGetD3D11DeviceContext()->OMSetBlendState(mTransparencyBS,blendFactor,0xffffffff);
			m_BlendState = BlendState::TransparencyBS;
		}
	}
	
	static void  SetNoCull()
	{
		DXUTGetD3D11DeviceContext()->RSSetState( m_pNoCullRS );
		m_RasterizerState = RasterizerState::NoCullRS;
	}

	static void SetDefaultCull()
	{
		if ( m_RasterizerState != RasterizerState::DefaultRS )
		{
			DXUTGetD3D11DeviceContext()->RSSetState( 0 );
			m_RasterizerState = RasterizerState::NoCullRS;
		}
	}

	static void ResetAllStates()
	{
		ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();
		float blendFactor[4]={0.0f,0.0f,0.0f,0.0f};
		context->OMSetBlendState( 0, blendFactor, 0xffffffff );
		context->OMSetDepthStencilState( mDefaultDSS, 0.0f );
		context->RSSetState( 0 );
		m_RasterizerState = RasterizerState::NoCullRS;
		m_BlendState = BlendState::DefaultBS;
		m_DepthStencilState = DepthStencilState::DefaultDSS;
	}

};

