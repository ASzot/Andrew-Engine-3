//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "WaterObject.h"

CWaterObject::CWaterObject( float posX, float posY, float posZ )
{
	CWaterObject( posX, posY, posZ, 1.0f, 1.0f, false );
}

CWaterObject::CWaterObject()
	: m_cPlaneModelName( "WaterPlane.obj" ),
	m_cCircleModelName( "CircularPlane.obj" ),
	m_bUseCircle( false ),
	m_fCurrentWaterTexTrans( 0.0f ),
	m_fDiffuseAbsorbity( 4.0f ),
	m_fSpecularAbsorbity( 1.0f )
{
}

CWaterObject::CWaterObject(float posX, float posY, float posZ, float scaleX, float scaleZ, bool useCircle)
{
	mPosX					= posX;
	mPosY					= posY;
	mPosZ					= posZ;
	mScale					= scaleX;
	mScaleZ					= scaleZ;
	m_fCurrentWaterTexTrans = 0.0f;
	m_fWaterTexTrans		= 0.001f;
	m_fWaterScale			= 0.1f;
	m_bUseCircle			= useCircle;
	m_cPlaneModelName		= "WaterPlane.obj";
	m_cCircleModelName		= "CircularPlane.obj";
	m_fDiffuseAbsorbity		= 4.0f;
	m_fSpecularAbsorbity		= 1.0f;
}


CWaterObject::~CWaterObject(void)
{
}


void CWaterObject::Render( ID3D11DeviceContext* context, CameraInfo& cam, const CEffectManager* effectManager, ExtraRenderingData& erd )
{
	D3DX11_TECHNIQUE_DESC techDesc;

	ID3D11Buffer* pVB = mMesh.GetVertexBuffer();
	UINT stride = sizeof( ObjectVertex );
	UINT offset = 0;

	context->IASetVertexBuffers( 0, 1, &pVB, &stride, &offset );
	context->IASetIndexBuffer( mMesh.GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0 );

	UpdateWorld();
	XMMATRIX view = cam.View;
	XMMATRIX proj = cam.Proj;
	XMMATRIX refl = cam.Refl;
	XMMATRIX world = XMLoadFloat4x4( &m_mWorld );
	XMMATRIX WVP = world * view * proj;

	HRESULT hr = effectManager->mWaterFX->SetWorld( world );
	Checkhr( hr, "Couldn't set WaterFX world" );
	hr = effectManager->mWaterFX->SetWorldViewProj( WVP );
	Checkhr( hr, "Couldn't set WaterFX WVP" );
	hr = effectManager->mWaterFX->SetReflectRefractScale( m_fWaterScale );
	Checkhr( hr, "Couldn't set WaterFX ReflectRefractScale" );
	hr = effectManager->mWaterFX->SetWaterTranslation( m_fCurrentWaterTexTrans );
	Checkhr( hr, "Couldn't set WaterFX WaterTrans" );
	hr = effectManager->mWaterFX->SetDiffuseAbsorbity( m_fDiffuseAbsorbity );
	hr = effectManager->mWaterFX->SetSpecularAbsorbity( m_fSpecularAbsorbity );

	XMMATRIX ReflProjWorld = XMMatrixTranspose( world ) * XMMatrixTranspose( refl ) * XMMatrixTranspose( proj );
	XMMATRIX viewProjWorld = world * view * proj;
	
	effectManager->mWaterFX->SetEyePos( cam.Position );
	
	hr = effectManager->mWaterFX->SetReflProjWorld( ReflProjWorld );
	hr = effectManager->mWaterFX->SetViewProjWorld( viewProjWorld );
	effectManager->SetInputLayout( InputLayoutType::PosNormTex );

	ID3DX11EffectTechnique* tech = effectManager->mWaterFX->WaterRefractTech;
	tech->GetDesc( &techDesc );
	for ( UINT subset = 0; subset < mMesh.GetNumberOfSubsets(); ++subset ) 
	{
		context->IASetPrimitiveTopology( mMesh.GetTopology( mRenderTech ) );

		for ( UINT p = 0; p < techDesc.Passes; ++p )
		{
			tech->GetPassByIndex( p )->Apply( 0, context );
			context->DrawIndexed( mMesh.GetIndexDrawAmount( subset ), mMesh.GetIndexStart( subset ), 0 );
		}
	}
}

bool CWaterObject::IsUsingCircle()
{
	return m_bUseCircle;
}


WaterSystemOptions CWaterObject::GetDlgInfo()
{
	WaterSystemOptions wso;
	wso.TargetedObjectsForRefraction = m_TargetedObjects;
	wso.x = X();
	wso.y = Y();
	wso.z = Z();
	wso.rx = RX();
	wso.ry = RY();
	wso.rz = RZ();
	wso.sx = S();
	wso.sy = mScaleY;
	wso.sz = mScaleZ;
	wso.UseCircle = m_bUseCircle;
	wso.TexTrans = m_fWaterTexTrans;
	wso.WaterScale = m_fWaterScale;
	wso.DiffuseAbsorbity = m_fDiffuseAbsorbity;
	wso.SpecularAbsorbity = m_fSpecularAbsorbity;

	return wso;
}

void CWaterObject::SetDlgInfo( WaterSystemOptions wso )
{
	m_TargetedObjects = wso.TargetedObjectsForRefraction;
	SetX( wso.x );
	SetY( wso.y );
	SetZ( wso.z );
	SetRX( wso.rx );
	SetRY( wso.ry );
	SetRZ( wso.rz );
	mScale = wso.sx;
	mScaleZ = wso.sz;
	// Check if the user wants a new model to be loaded.
	if ( m_bUseCircle != wso.UseCircle )
	{
		BulletData bd;
		bd.Mat = PhysMat_Normal;
		bd.SpecGrav = 0.0f;
		const char* modelName = wso.UseCircle ? m_cCircleModelName : m_cPlaneModelName;

		// Restart the particle system.
		DestroyResources();
		Shutdown();
		Initialize( DXUTGetD3D11Device(), modelName, eBASIC, bd );
	}
	m_bUseCircle = wso.UseCircle;

	m_fWaterScale = wso.WaterScale;
	m_fWaterTexTrans = wso.TexTrans;
	m_fDiffuseAbsorbity = wso.DiffuseAbsorbity;
	m_fSpecularAbsorbity = wso.SpecularAbsorbity;
}
	
std::vector<ObjectId> CWaterObject::GetTargetedObjects()
{
	return m_TargetedObjects;
}


void CWaterObject::SetWaterScale( float waterScale )
{
	m_fWaterScale = waterScale;
}

void CWaterObject::SetWaterTexTrans( float waterTexTrans )
{
	m_fWaterTexTrans = waterTexTrans;
}

float CWaterObject::GetWaterScale()
{
	return m_fWaterScale;
}

float CWaterObject::GetWaterTexTrans()
{
	return m_fWaterTexTrans;
}

void CWaterObject::UpdateWaterTrans()
{
	m_fCurrentWaterTexTrans += m_fWaterTexTrans;
	if ( m_fCurrentWaterTexTrans > 1.0f )
	{
		m_fCurrentWaterTexTrans -= 1.0f;
	}
}


float CWaterObject::GetDiffuseAb()
{
	return m_fDiffuseAbsorbity;
}

float CWaterObject::GetSpecAb()
{
	return m_fSpecularAbsorbity;
}