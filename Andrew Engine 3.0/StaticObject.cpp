//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "StaticObject.h"
#include "LightManager.h"


CStaticObject::CStaticObject() : IObject()
{
	mTexScaleX = 1.0f;
	mTexScaleY = 1.0f;
	mTexScaleZ = 1.0f;
	mPosX = 0;
	mPosY = 5.0f;
	mPosZ = 0;
	mRotX = 0;
	mRotY = 0;
	mRotZ = 0;
	mTexTransX = 0;
	mTexTransY = 0;
	mTexTransZ = 0;
	m_iFlashing = 0;

	mScale = 1.0f;
	mScaleY = 1.0f;
	mScaleZ = 1.0f;
	mRenderTech = eBASIC;

	m_RenderTechData.Reflections = false;
	m_RenderTechData.Refractable = false;
	m_iFlashingCount = 0;
}

CStaticObject::CStaticObject(D3DXVECTOR3 pos, D3DXVECTOR3 rot,XMFLOAT3 scale,XMFLOAT3 texScale,XMFLOAT3 texTrans)
	: IObject()
{
	mPosX = pos.x;
	mPosY = pos.y;
	mPosZ = pos.z;
	mRotX = rot.x;
	mRotY = rot.y;
	mRotZ = rot.z;
	mScale = scale.x;
	mScaleY = scale.y;
	mScaleZ = scale.z;

	mTexScaleX = texScale.x;
	mTexScaleY = texScale.y;
	mTexScaleZ = texScale.z;

	mTexTransX = texTrans.x;
	mTexTransY = texTrans.y;
	mTexTransZ = texTrans.z;

	m_RenderTechData.Reflections = false;
	m_RenderTechData.Refractable = false;

	m_iFlashingCount = 0;
}

CStaticObject::~CStaticObject()
{

}

bool CStaticObject::Initialize(ID3D11Device* device,const std::string filename, RenderTech renderTech, BulletData data)
{
	RenderTechData rtd;
	rtd.Reflections = false;
	rtd.Refractable = false;

	return Initialize( device, filename, renderTech, data, rtd );
}

bool CStaticObject::Initialize(ID3D11Device* device, const std::string filename, RenderTech renderTech, BulletData data, RenderTechData rtd)
{
	m_RenderTechData.Reflections = rtd.Reflections;
	m_RenderTechData.Refractable = rtd.Refractable;
	mRenderTech = renderTech;
	m_BulletData = data;
	
	// Used for loading normal maps.
	WStringList textureNames;
	if(!mMesh.Initialize("data/Models/"+filename,device,textureNames))
	{
		std::string s = "Couldn't open " + s;
		DebugWriter::WriteToDebugFile( s.c_str(), "bool CStaticObject::Initialize(ID3D11Device* device, const std::string filename, RenderTech renderTech, BulletData data)");
		return false;
	}


	mModelName = std::string(filename.begin(),filename.end() - 4);

	UpdateWorld();
	CreateBoundingBox();

	return true;
}

void CStaticObject::Shutdown()
{
	mMesh.Shutdown();
}

void CStaticObject::DestroyResources()
{
	mMesh.DestroyResources();
}

void CStaticObject::Render( ID3D11DeviceContext* context, CameraInfo& cam,const CEffectManager* effectManager, ExtraRenderingData& erd )
{
	ID3D11Buffer* pVB = mMesh.GetVertexBuffer();
	UINT stride = sizeof(ObjectVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0,1,&pVB,&stride,&offset);
	context->IASetIndexBuffer(mMesh.GetIndexBuffer(),DXGI_FORMAT_R32_UINT,0);

	// Ensure that our world matrix is synced up with that of the physics system.
	UpdateWorld();

	XMMATRIX world,texWorld,view,proj,WVP,inverseTranspose;
	world = XMLoadFloat4x4(&m_mWorld);
	texWorld = XMMatrixScaling ( mTexScaleX, mTexScaleY, mTexScaleZ ) * XMMatrixTranslation( mTexTransX, mTexTransY, mTexTransZ );
	view = cam.View;
	proj = cam.Proj;
	WVP = world * view * proj;
	inverseTranspose = SMathHelper::InverseTranspose(world);

	CBasicEffectRenderInfo info; 
	// Set all of the effect
	info.eyePosW = cam.Position;
	info.fogColor = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
	info.fogStart = 15.0f;
	info.fogRange = 175.0f;
	info.proj = proj;
	info.view = view;
	info.world = world;
	// Initialize all the stuff we don't support yet to just random stuff
	info.texTransform = texWorld;

	// Transform each shadow transform into the objects actual space.
	for ( int i = 0; i < m_ShadowTransforms.size(); ++i )
	{
		XMMATRIX mt = XMLoadFloat4x4( &m_ShadowTransforms.at( i ) );
		mt = world * mt;
		XMStoreFloat4x4( &m_ShadowTransforms.at( i ), mt );
	}

	//TODO:
	// Change how a state gets set without it even being used.
	// A good solution would to make normal mapping and basic lighting in the 
	// same file as they are so closely related. ( Compared to displacement mapping 
	// which uses completely different data. The state setting of the displacement
	// mapping is also redundant.

	// Set the normal map per object stuff.
	NormalMappingExtraInfo et( 0.07f, 0.5f, 25.0f, 1.0f, 10.0f );
	if ( !effectManager->SetNormalMappingFX( info, et, context ) )
		FatalError( "Couldn't set the normal mapping fx!" );
	

	// Set the basic effect stuff.
	if ( !effectManager->SetBasicFX( info,context ) )
		FatalError("Couldnt set something in basic fx!");


#pragma region SetFlashing
	double time = DXUTGetTime();
	if ( m_iFlashing == GREEN_FLASH_COLOR )
	{
		if ( ( ( (int)time % 2 % 180 ) == 0 ) || ( m_iFlashingCount != 0 ) )
		{
			effectManager->mBasicFX->SetFlashing( true );
			effectManager->mNormalMapFX->SetFlashing( true );
			++m_iFlashingCount;
			if ( m_iFlashingCount > 15 )
				m_iFlashingCount = 0;
		}
			
	}
	else
	{
		effectManager->mBasicFX->SetFlashing( false );
		effectManager->mNormalMapFX->SetFlashing( false );
	}
#pragma endregion


	
	effectManager->mBasicFX->SetShadowTransforms( m_ShadowTransforms );
	effectManager->mNormalMapFX->SetShadowTransforms( m_ShadowTransforms );

	
	bool hasTexture;
	
	for( UINT subset = 0; subset < mMesh.GetNumberOfSubsets(); ++subset)
	{
		SRenderSettings::SetDefaultBS();
		bool hasNormalMap = mMesh.GetHasNormalMap( subset );
		hasTexture = mMesh.GetHasTexture(subset);
		context->IASetPrimitiveTopology(mMesh.GetTopology(mRenderTech));

		if ( mMesh.GetIsTransparent( subset ) )
			SRenderSettings::SetTransparencyBlending( mMesh.GetTransparency( subset ) );

		ID3DX11EffectTechnique* activeTech = effectManager->GetTechnique( mRenderTech, hasNormalMap, true, m_RenderTechData.Reflections, false );
		D3DX11_TECHNIQUE_DESC techDesc;
		activeTech->GetDesc(&techDesc);

		if ( !hasNormalMap )		// Set all of the basic effect specific information.
		{
			effectManager->SetInputLayout( InputLayoutType::PosNormTex );

			effectManager->SetBasicFXMaterial(mMesh.GetSubsetMaterial(subset));

			effectManager->SetBasicFXHasTexture(hasTexture);

			if(hasTexture)
			{
				ID3D11ShaderResourceView* pDiffuseSRV = mMesh.GetSubsetDiffuseMapSRV(subset);
				effectManager->SetBasicFXDiffuseMap(pDiffuseSRV);
			}
		}
		else						// Set all of the normal map specific information.
		{
			effectManager->SetInputLayout( InputLayoutType::PosNormTexTan );

			if(!effectManager->SetNormalMappingHasTexture(hasTexture))
				FatalError("LOL");

			if(hasTexture)
			{
				ID3D11ShaderResourceView* pDiffuseSRV = mMesh.GetSubsetDiffuseMapSRV( subset );
				if( !effectManager->SetNormalMappingDiffuseMap( pDiffuseSRV ) )
					FatalError( "LOL" );
				ID3D11ShaderResourceView* pNormalSRV = mMesh.GetSubsetNormalMapSRV( subset );
				if (!effectManager->SetNormalMappingNormalMap( pNormalSRV ) )
					DebugWriter::WriteToDebugFile( L"Couldn't set the normal map!" );
			}
		}
		
		
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			activeTech->GetPassByIndex(p)->Apply( 0, context );
			context->DrawIndexed( mMesh.GetIndexDrawAmount( subset ), mMesh.GetIndexStart( subset ), 0 );
		}
	}

	m_ShadowTransforms.clear();
}


void CStaticObject::RenderShadowMap(ID3D11DeviceContext* context, CameraInfo& cam,const CEffectManager* effectManager, CLightManager* pLightMgr)
{

	UpdateWorld();

	ID3D11Buffer* pVB = mMesh.GetVertexBuffer();
	UINT stride = sizeof(ObjectVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0,1,&pVB,&stride,&offset);
	context->IASetIndexBuffer(mMesh.GetIndexBuffer(),DXGI_FORMAT_R32_UINT,0);

	effectManager->SetInputLayout(InputLayoutType::PosNormTex);
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// These should be the light view and proj.
	Vec position = Vec( X(), Y(), Z() );
	XMMATRIX view = cam.View;
	XMMATRIX proj = cam.Proj;
	XMMATRIX viewProj = view * proj;
	XMMATRIX world = XMLoadFloat4x4( &m_mWorld );

	effectManager->mShadowFX->SetEyePosW( cam.Position );
	effectManager->mShadowFX->SetTexTransform( XMMatrixIdentity() );
	effectManager->mShadowFX->SetViewProj( viewProj );
	effectManager->mShadowFX->SetWorld( world );
	effectManager->mShadowFX->SetWorldInvTranspose( SMathHelper::InverseTranspose( world ) );
	effectManager->mShadowFX->SetWorldViewProj( world * view * proj );

	ID3DX11EffectTechnique* activeTech = effectManager->mShadowFX->BuildShadowMapTech;
	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc( &techDesc );

	for( UINT subset = 0; subset < mMesh.GetNumberOfSubsets(); ++subset)
	{	
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			activeTech->GetPassByIndex( p )->Apply( 0, context );
			context->DrawIndexed( mMesh.GetIndexDrawAmount( subset ), mMesh.GetIndexStart( subset ), 0 );
		}
	}

	// Cache the shadow transform, for when we will be needing it in the actual render.
	XMMATRIX shadowTransform = pLightMgr->ConstructShadowTransform( view, proj );
	XMFLOAT4X4 temp;
	XMStoreFloat4x4( &temp, shadowTransform );
	m_ShadowTransforms.push_back( temp );
}


void CStaticObject::Translate(float x, float y, float z)
{
	mPosX = x;
	mPosY = y;
	mPosZ = z;
}

void CStaticObject::Translate(XMVECTOR dir)
{
	//Implement later
}

void CStaticObject::MoveX(float dir)
{
	mPosX = mPosX + dir;
}

void CStaticObject::MoveY(float dir)
{
	mPosY = mPosY + dir;
}

void CStaticObject::MoveZ(float dir)
{
	mPosZ = mPosZ + dir;
}

void CStaticObject::RotateZ(float d)
{
	mRotZ += d;
}

void CStaticObject::RotateX(float d)
{
	mRotX += d;
}

void CStaticObject::RotateY(float d)
{
	mRotY += d;
}

void CStaticObject::Scale(float d)
{
	mScale += d;
}

void CStaticObject::TexScale(float d)
{
	mTexScaleX += d;
	mTexScaleY += d;
	mTexScaleZ += d;
}

void CStaticObject::Serilize(CDataArchiver* archiver)
{
	if ( !GetIsSerilizible() )
		return;
	std::string finalMName = mModelName + ".obj";
	archiver->WriteToStream( 'm' );
	archiver->Space();
	archiver->WriteToStream( finalMName );
	archiver->Space();
	archiver->WriteToStream( mPosX );
	archiver->Space();
	archiver->WriteToStream( mPosY );
	archiver->Space();
	archiver->WriteToStream( mPosZ );
	archiver->Space();
	archiver->WriteToStream( mRotX );
	archiver->Space();
	archiver->WriteToStream( mRotY );
	archiver->Space();
	archiver->WriteToStream( mRotZ );
	archiver->Space();
	archiver->WriteToStream( XMFLOAT3( mScale, mScaleY, mScaleZ ) );
	archiver->Space();
	archiver->WriteToStream( mTexScaleX  );
	archiver->Space();
	archiver->WriteToStream( mTexScaleY );
	archiver->Space();
	archiver->WriteToStream( mTexScaleZ );
	archiver->Space();
	archiver->WriteToStream( XMFLOAT3( mTexTransX, mTexTransY, mTexTransZ ) );
	archiver->Space();
	archiver->WriteToStream( m_BulletData.SpecGrav );
	archiver->Space();
	archiver->WriteToStream( (int)( mRenderTech ) );
	archiver->Space();
	archiver->WriteToStream( (int)( m_BulletData.Mat ) );
	archiver->Space();
	archiver->WriteToStream( m_RenderTechData.Reflections );
	archiver->Space();
	archiver->WriteToStream( m_RenderTechData.Refractable );
	archiver->WriteToStream('\n');
}


void CStaticObject::CreateBoundingBox()
{
	UpdateWorld();
	// Create without the scale since bullet doesn't like scale.
	XMMATRIX translation, rotX, rotY, rotZ, scale;
	translation = XMMatrixTranslation( mPosX, mPosY, mPosZ );
	rotX = XMMatrixRotationX( XMConvertToRadians( mRotX ) );
	rotY = XMMatrixRotationY( XMConvertToRadians( mRotY ) );
	rotZ = XMMatrixRotationZ( XMConvertToRadians( mRotZ ) );
	scale = XMMatrixScaling(mScale,mScaleY,mScaleZ);
	XMMATRIX m = rotX * rotY * rotZ * translation;
	mBoundingBox = Collision::CalculateBoundingBox(mMesh.GetVertices(),m);
}

void CStaticObject::UpdateWorld(void)
{
	XMMATRIX translation, rotX, rotY, rotZ, axisRot, scale;
	axisRot = XMMatrixIdentity();
	translation = XMMatrixTranslation( mPosX, mPosY, mPosZ );
	rotX = XMMatrixRotationX( XMConvertToRadians( mRotX ) );
	rotY = XMMatrixRotationY( XMConvertToRadians( mRotY ) );
	rotZ = XMMatrixRotationZ( XMConvertToRadians(mRotZ) );
	if ( m_f4AxisRot.w != 0.0f )	
		axisRot  = XMMatrixRotationAxis( XMVectorSet( m_f4AxisRot.x, m_f4AxisRot.y, m_f4AxisRot.z, 0.0f ), m_f4AxisRot.w );
	scale = XMMatrixScaling(mScale,mScaleY,mScaleZ);
	// Remember scale rotation translation (SRT).
	XMMATRIX final = scale * rotX * rotY * rotZ * axisRot * translation;
	XMStoreFloat4x4(&m_mWorld,final);
}

Mat4x4 CStaticObject::GetWorldMatrix()
{
	// Not the fastest but it has to be done.
	// We can't include scale in the matrix we pass on to bullet.

	//TODO:
	// Try to just load the world matrix stored as the float4x4.
	XMMATRIX translation, rotX, rotY, rotZ;
	translation = XMMatrixTranslation( mPosX, mPosY, mPosZ );
	rotX = XMMatrixRotationX( XMConvertToRadians( mRotX ) );
	rotY = XMMatrixRotationY( XMConvertToRadians( mRotY ) );
	rotZ = XMMatrixRotationZ( XMConvertToRadians( mRotZ ) );
	XMMATRIX m = rotX * rotY * rotZ * translation;
	D3DXMATRIX* dx = (D3DXMATRIX*)&m;
	Mat4x4 r(*dx);
	return r;
}

void CStaticObject::SetWorldMatrix(Mat4x4& m)
{
	IObject::SetWorldMatrix(m);
	XMStoreFloat4x4(&m_mWorld,m.ToXMMatrix());
}

void CStaticObject::SetWorldMatrixXM(CXMMATRIX m)
{
	XMStoreFloat4x4( &m_mWorld, m );
}


void CStaticObject::SetRenderData(RenderTechData rtd)
{
	m_RenderTechData = rtd;
}

RenderTechData CStaticObject::GetRenderData()
{
	return m_RenderTechData;
}

void CStaticObject::SetPositionData(StaticObjectData d)
{
	mPosX = d.x / STATIC_OBJECT_DATA_FACTOR;
	mPosY = d.y / STATIC_OBJECT_DATA_FACTOR;
	mPosZ = d.z / STATIC_OBJECT_DATA_FACTOR;

	mRotX = d.rx / STATIC_OBJECT_DATA_FACTOR;
	mRotY = d.ry / STATIC_OBJECT_DATA_FACTOR;
	mRotZ = d.rz / STATIC_OBJECT_DATA_FACTOR;

	mScale = d.s / STATIC_OBJECT_DATA_FACTOR;
	mScaleY = d.sy / STATIC_OBJECT_DATA_FACTOR;
	mScaleZ = d.sz / STATIC_OBJECT_DATA_FACTOR;

	mTexScaleX = d.tx / STATIC_OBJECT_DATA_FACTOR;
	mTexScaleY = d.ty / STATIC_OBJECT_DATA_FACTOR;
	mTexScaleZ = d.tz / STATIC_OBJECT_DATA_FACTOR;

	mTexTransX = d.ttx / STATIC_OBJECT_DATA_FACTOR;
	mTexTransY = d.tty / STATIC_OBJECT_DATA_FACTOR;
	mTexTransZ = d.ttz / STATIC_OBJECT_DATA_FACTOR;

	float t = (float)d.opacity;
	mMesh.SetOpacity(t / 100.0f);
}

void CStaticObject::RenderRefraction( ID3D11DeviceContext* context, CameraInfo& cam, CEffectManager* effectManager )
{
	D3DX11_TECHNIQUE_DESC techDesc;

	ID3D11Buffer* pVB = mMesh.GetVertexBuffer();
	UINT stride = sizeof( ObjectVertex );
	UINT offset = 0;

	context->IASetVertexBuffers( 0, 1, &pVB, &stride, &offset );
	context->IASetIndexBuffer( mMesh.GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0 );

	//UpdateWorld();

	XMMATRIX world = XMLoadFloat4x4( &m_mWorld );
	XMMATRIX WVP = world * cam.View * cam.Proj;
	XMMATRIX worldInvTranspose = SMathHelper::InverseTranspose( world );
	
	HRESULT hr;
	hr = effectManager->mRefractFX->SetWorldViewProj( WVP );
	hr = effectManager->mRefractFX->SetWorld( world );
	hr = effectManager->mRefractFX->SetWorldInvTranspose( worldInvTranspose );
	hr = effectManager->mRefractFX->SetEyePos( cam.Position );

	effectManager->SetInputLayout( InputLayoutType::PosNormTex );

	ID3DX11EffectTechnique* tech = effectManager->mRefractFX->RefractWithBasicLighting;
	tech->GetDesc( &techDesc );
	for ( UINT subset = 0; subset < mMesh.GetNumberOfSubsets(); ++subset ) 
	{
		effectManager->mRefractFX->SetDiffuseMap( mMesh.GetSubsetDiffuseMapSRV(subset) );
		context->IASetPrimitiveTopology( mMesh.GetTopology( mRenderTech ) );


		for ( UINT p = 0; p < techDesc.Passes; ++p )
		{
			tech->GetPassByIndex( p )->Apply( 0, context );
			context->DrawIndexed( mMesh.GetIndexDrawAmount( subset ), mMesh.GetIndexStart( subset ), 0 );
		}
	}
}

void CStaticObject::SetFlashing( int flashing )
{
	m_iFlashing = flashing;
}

void CStaticObject::OnLeftClicked(UINT flags)
{
	
}

tPoints CStaticObject::GetVertices()
{
	return mMesh.GetVertices();
}

StaticObjectData CStaticObject::GetPositionData()
{
	StaticObjectData data(mPosX * STATIC_OBJECT_DATA_FACTOR,
		mPosY * STATIC_OBJECT_DATA_FACTOR,
		mPosZ * STATIC_OBJECT_DATA_FACTOR,

		mRotX * STATIC_OBJECT_DATA_FACTOR,
		mRotY * STATIC_OBJECT_DATA_FACTOR,
		mRotZ * STATIC_OBJECT_DATA_FACTOR,

		mScale * STATIC_OBJECT_DATA_FACTOR,
		mScaleY * STATIC_OBJECT_DATA_FACTOR,
		mScaleZ * STATIC_OBJECT_DATA_FACTOR,

		mTexScaleX * STATIC_OBJECT_DATA_FACTOR,
		mTexScaleY * STATIC_OBJECT_DATA_FACTOR,
		mTexScaleZ * STATIC_OBJECT_DATA_FACTOR,

		mTexTransX * STATIC_OBJECT_DATA_FACTOR,
		mTexTransY * STATIC_OBJECT_DATA_FACTOR,
		mTexTransZ * STATIC_OBJECT_DATA_FACTOR,
		(int)((mMesh.GetOpacity())*100.0f));
	return data;
}

Vec CStaticObject::GetBoundriesOfCollisionBox()
{
	Collision::BoundingBox* box = GetBoundingBox();
	float w,l,h;
	// Width is x, length is z, and h is y.
	SMathHelper::GetMinAndMaxAsBox(w,h,l,box->MinVertex,box->MaxVertex);

	// Scale already taken into consideration.
	return Vec( w, h, l );
}

ObjectPhysicsPropData CStaticObject::GetPhysicsProperties()
{
	ObjectPhysicsPropData d;
	d.SpecGrav = GetSpecGrav();
	d.Material = (int)GetPhysicsMaterial();

	return d;
}

void CStaticObject::SetAllCoordinates( PositionData data )
{
	IObject::SetAllCoordinates( data );
	mScaleY = data.sy;
	mScaleZ = data.sz;
}

void CStaticObject::SetPos( float x,float y,float z )
{
	mPosX = x;
	mPosY = y;
	mPosZ = z;
}

Collision::BoundingBox* CStaticObject::GetBoundingBox()
{
	return &mBoundingBox;
}

RenderTech CStaticObject::GetRenderTech()
{
	return mRenderTech;
}

void CStaticObject::SetOpacity( float opacity )
{
	mMesh.SetOpacity(opacity);
}

void CStaticObject::SetRenderTech( RenderTech renderTech )
{
	mRenderTech = renderTech;
}
