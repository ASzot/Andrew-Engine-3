//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "AnimatedObject.h"
#include "LightManager.h"
#include "System.h"

CAnimatedObject::CAnimatedObject()
{
	// Default speed
	m_fSpeedOfAnimation		= 1.0f;

	m_vCurrCharDirection	= XMFLOAT4(0.0f,0.0f,0.0f,0.0f);
	m_vOldCharDirection		= XMFLOAT4(0.0f,0.0f,0.0f,0.0f);
	m_vCharPosition			= XMFLOAT4(0.0f,0.0f,0.0f,0.0f);
	m_bMove = false;
	m_fExtraPhysicsHeight = 1.4f;
}


CAnimatedObject::~CAnimatedObject()
{

}

bool CAnimatedObject::Init(PositionData data)
{
	// Don't forget we need the same code in both of the init functions, the overloaded version AND this one.
	mPosX = data.x;
	mPosY = data.y;
	mPosZ = data.z;
	mRotX = data.rx;
	mRotY = data.ry;
	mRotZ = data.rz;
	mScale = data.s;
	if(data.sy == 0.0f)
	{
		mScaleY = data.s;
		mScaleZ = data.z;
		return true;
	}
	mScaleY = data.sy;
	mScaleZ = data.sz;

	//m_pController = new CAnimObjController(&mPosX,&mPosY,&mPosZ,&mRotX,&mRotY,&mRotZ);
	//assert(m_pController);

	return true;
}

bool CAnimatedObject::Init(PositionData* data)
{
	mPosX = data->x;
	mPosY = data->y;
	mPosZ = data->z;
	mRotX = data->rx;
	mRotY = data->ry;
	mRotZ = data->rz;
	mScale = data->s;

	if(data->sy == 0.0f)
	{
		mScaleY = data->s;
		mScaleZ = data->z;
		return true;
	}
	mScaleY = data->sy;
	mScaleZ = data->sz;

	delete data;
	data = 0;

	return true;
}

bool CAnimatedObject::InitResources(ID3D11Device* device,const char* filename,const wchar_t* textureDir,char* clipName,TextureMgr& textureManager)
{
	std::ifstream fin( filename );
	if ( !fin )
		return false;
	mCharacterModel = new SkinnedModel( device, textureManager, filename, textureDir );
	assert( mCharacterModel );
	mCharacterInstance.Model = mCharacterModel;
	mCharacterInstance.TimePos = 0.0f;
	mCharacterInstance.ClipName = clipName;
	mCharacterInstance.FinalTransforms.resize( mCharacterModel->SkinnedData.BoneCount() );


	return true;
}


void CAnimatedObject::Shutdown(void)
{
	
}


void CAnimatedObject::Update(float dt)
{
	if(dt == -1.0f)
	{
		mCharacterInstance.Update(0.0001f);
	}
	if(m_bMove)
	{
		Animate( m_vDesiredDir, dt );
		m_bMove = false;
	}
	UpdateWorldMatrix();
}


void CAnimatedObject::Render(CameraInfo& cam, CSkinEffect* effect)
{
	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ID3DX11EffectTechnique* renderTech = effect->LightTexShadowSkin;
	D3DX11_TECHNIQUE_DESC techDesc;
	renderTech->GetDesc(&techDesc);
	XMMATRIX world;
	XMMATRIX worldViewProj;
	XMMATRIX worldInvTranspose;
	XMMATRIX tex = XMMatrixIdentity();
	XMMATRIX view = cam.View;
	XMMATRIX proj = cam.Proj;

	world = XMLoadFloat4x4(&mCharacterInstance.World);
	

	// Transform each shadow transform into the objects actual space.
	for ( int i = 0; i < m_ShadowTransforms.size(); ++i )
	{
		XMMATRIX mt = XMLoadFloat4x4( &m_ShadowTransforms.at( i ) );
		mt = world * mt;
		XMStoreFloat4x4( &m_ShadowTransforms.at( i ), mt );
	}

	effect->SetShadowTransforms( m_ShadowTransforms );

	for(int p = 0; p < techDesc.Passes; ++p)
	{
		
		worldViewProj = world * view * proj;
		worldInvTranspose = SMathHelper::InverseTranspose(world);

		effect->World->SetMatrix(reinterpret_cast<const float*>(&world));
		effect->WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&worldInvTranspose));
		effect->WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&worldViewProj));
		effect->TexTransform->SetMatrix(reinterpret_cast<const float*>(&tex));
		effect->BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(&(mCharacterInstance.FinalTransforms[0])),0,mCharacterInstance.FinalTransforms.size());
		
		for(UINT subset = 0; subset < mCharacterInstance.Model->SubsetCount; ++subset)
		{
			effect->Mat->SetRawValue(&mCharacterInstance.Model->Mat[subset],0,sizeof(Material));
			effect->DiffuseMap->SetResource(mCharacterInstance.Model->DiffuseMapSRV[subset]);
			effect->NormalMap->SetResource(mCharacterInstance.Model->NormalMapSRV[subset]);

			renderTech->GetPassByIndex(p)->Apply(0,context);
			mCharacterInstance.Model->ModelMesh.Draw(context,subset);
		}
	}

	// Unbind the OM Depth Stencil as input.
	effect->ShadowMap->SetResource( NULL );

	m_ShadowTransforms.clear();
}

void CAnimatedObject::RenderShadowMap( CLightManager* lightMgr, CameraInfo& cam, CSkinEffect* effect )
{
	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ID3DX11EffectTechnique* renderTech = effect->BuildShadowMap;
	D3DX11_TECHNIQUE_DESC techDesc;
	renderTech->GetDesc(&techDesc);
	XMMATRIX world;
	XMMATRIX worldViewProj;
	XMMATRIX worldInvTranspose;
	XMMATRIX tex = XMMatrixIdentity();
	XMMATRIX view = cam.View;
	XMMATRIX proj = cam.Proj;

	effect->ShadowMap->SetResource( NULL );
	effect->ShadowTransform->SetMatrix( reinterpret_cast<const float*>( &XMMatrixIdentity() ) );

	for(int p = 0; p < techDesc.Passes; ++p)
	{
		world = XMLoadFloat4x4(&mCharacterInstance.World);
		worldViewProj = world * view * proj;
		worldInvTranspose = SMathHelper::InverseTranspose(world);

		effect->World->SetMatrix(reinterpret_cast<const float*>(&world));
		effect->WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&worldInvTranspose));
		effect->WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&worldViewProj));
		effect->TexTransform->SetMatrix(reinterpret_cast<const float*>(&tex));
		effect->BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(&(mCharacterInstance.FinalTransforms[0])),0,mCharacterInstance.FinalTransforms.size());
		
		for(UINT subset = 0; subset < mCharacterInstance.Model->SubsetCount; ++subset)
		{
			effect->Mat->SetRawValue(&mCharacterInstance.Model->Mat[subset],0,sizeof(Material));
			effect->DiffuseMap->SetResource( NULL );
			effect->NormalMap->SetResource( NULL );

			renderTech->GetPassByIndex(p)->Apply(0,context);
			mCharacterInstance.Model->ModelMesh.Draw(context,subset);
		}
	}

	XMMATRIX shadowTransform = lightMgr->ConstructShadowTransform( view, proj );
	XMFLOAT4X4 temp;
	XMStoreFloat4x4( &temp, shadowTransform );
	m_ShadowTransforms.push_back( temp );
}


void CAnimatedObject::DestroyResources(void)
{
	SafeDelete(mCharacterModel);
}


void CAnimatedObject::Serilize(CDataArchiver* dataArchiver)
{
}


void CAnimatedObject::Animate(Vec& dir, float dt)
{
	XMVECTOR destinationDirection = dir.ToVector();
	XMVECTOR oldCharDirection = XMLoadFloat4(&m_vOldCharDirection);
	XMVECTOR charPosition = XMLoadFloat4(&m_vCharPosition);
	XMVECTOR currCharDirection = XMLoadFloat4(&m_vCurrCharDirection);

	XMVECTOR defaultForward = XMVectorSet(0.0f,0.0f,1.0f,0.0f);
	destinationDirection = XMVector3Normalize(destinationDirection);

	// To make the character not take forever when turning around.
	if(XMVectorGetX(XMVector3Dot(destinationDirection,oldCharDirection)) == -1)
		oldCharDirection += XMVectorSet(0.02f,0.0f,-0.02f,0.0f);

	float destDirLength = 10.0f * dt;

	currCharDirection = oldCharDirection + (destinationDirection * destDirLength);

	currCharDirection = XMVector3Normalize(currCharDirection);

	float charDirAngle = XMVectorGetX(XMVector3AngleBetweenNormals(XMVector3Normalize(currCharDirection),XMVector3Normalize(defaultForward)));
	if(XMVectorGetY(XMVector3Cross(currCharDirection,defaultForward)) > 0.0f)
		charDirAngle = -charDirAngle;


	float speed = 2.0f * dt;
	charPosition = charPosition + (destinationDirection * speed);


	mRotY = charDirAngle;

	XMStoreFloat4(&m_vCurrCharDirection,currCharDirection);
	m_vOldCharDirection = m_vCurrCharDirection;
	mCharacterInstance.Update(dt * m_fSpeedOfAnimation);
}

void CAnimatedObject::SetDirection( Vec& dir )
{
	m_vDesiredDir = dir;
	m_bMove = true;
}


// Be sure to call whenever the orientation or any of the position data of the object changes.
void CAnimatedObject::UpdateWorldMatrix(void)
{
	XMMATRIX scale,rotationX,rotationY,rotationZ,trans,final;
	scale = XMMatrixScaling(mScale,mScaleY,mScaleZ);
	rotationX = XMMatrixRotationX(mRotX);
	rotationY = XMMatrixRotationY(mRotY);
	rotationZ = XMMatrixRotationZ(mRotZ);
	trans = XMMatrixTranslation(mPosX,mPosY - m_fExtraPhysicsHeight,mPosZ);
	final = scale * rotationX * rotationY * rotationZ * trans;
	XMStoreFloat4x4(&mCharacterInstance.World,final);

	// The bounding box needs to be recreated every time our orientation changes.
	CreateBoundingBox();
}


void CAnimatedObject::CreateBoundingBox(void)
{
	XMMATRIX rotationX,rotationY,rotationZ,trans,final;
	rotationX = XMMatrixRotationX(mRotX);
	rotationY = XMMatrixRotationY(mRotY);
	rotationZ = XMMatrixRotationZ(mRotZ);
	trans = XMMatrixTranslation(mPosX,mPosY - m_fExtraPhysicsHeight,mPosZ);
	final = rotationX * rotationY * rotationZ * trans;

	mBoundingBox = Collision::CalculateBoundingBox( mCharacterInstance.GetVertices(), final );
}


Mat4x4 CAnimatedObject::GetWorldMatrix()
{
	XMMATRIX m = XMLoadFloat4x4(&mCharacterInstance.World);
	m._42 += m_fExtraPhysicsHeight;
	D3DXMATRIX* dx = (D3DXMATRIX*)&m;
	Mat4x4 r(*dx);
	return r;
}

void CAnimatedObject::SetWorldMatrix(Mat4x4& m)
{
	m._42 -= m_fExtraPhysicsHeight;
	IObject::SetWorldMatrix(m);

	XMMATRIX xm = m.ToXMMatrix();
	XMStoreFloat4x4(&mCharacterInstance.World,xm);
}

void CAnimatedObject::OnLeftClicked(UINT flags)
{
	// If there is a game representation of the object then send it the event.
	if ( m_pGameActor )
	{
		m_pGameActor->OnRayHit( flags );
		g_GetEventManager()->GetSystem()->DisplayConsoleText( std::wstring( L"Animated object hit!" ) );
	}
}