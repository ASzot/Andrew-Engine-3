//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "LightManager.h"



CLightManager::CLightManager(EGraphicsSettings settings) : mIsSceneLoaded(false), mWindow(), mSelectedLight(0), mEventListener(eInputEventType)
{
	//TODO: 
	// Make multiple graphics settings possible
	mEventListener.SetSelectedLight(mSelectedLight);

	m_fLightRotationAngle = 0.0f;
}


CLightManager::~CLightManager(void)
{
}


bool CLightManager::Initialize(HINSTANCE instance,HWND wnd, CEventManager* eventManager)
{
	mhInstance = instance;
	mhWnd = wnd;
	eventManager->RegisterListener(&mEventListener);

	eventManager->RegisterLightMgr( this );
	return true;
}

void CLightManager::Render(ID3D11DeviceContext* context,CEffectManager*& effect,CameraInfo& cam,CSkinEffect* skinEffect)
{
	if(!mIsSceneLoaded)
		FatalError("The lighting isn't loaded!");
	
	// Add the point lights from the shadow point lights.
	CSceneLighting sl;
	sl = mLightingInfo;
	for ( std::vector<CShadowPointLight>::iterator i = m_ShadowPointLights.begin(); i < m_ShadowPointLights.end(); ++i )
	{
		std::vector<PointLight> lights = ( *i ).GetPointLights();
		for ( std::vector<PointLight>::iterator pi = lights.begin(); pi < lights.end(); ++pi )
		{
			sl.pointLights.push_back( ( *pi ) );
		}
	}

	if(!effect->SetAllLighting(sl, skinEffect))
		FatalError("Can't set the lighting!");
}

void CLightManager::Serilize(CDataArchiver* archiver)
{
	for ( std::vector<PointLight>::iterator i = mLightingInfo.pointLights.begin(); i != mLightingInfo.pointLights.end(); ++i )
	{
		PointLight temp = *i;
		archiver->WriteToStream('l');
		archiver->Space();
		archiver->WriteFloat3ToStream( temp.Ambient );
		archiver->Space();
		archiver->WriteFloat3ToStream( temp.Diffuse );
		archiver->Space();
		archiver->WriteToStream( temp.Att );
		archiver->Space();
		archiver->WriteToStream( temp.Range );
		archiver->Space();
		archiver->WriteFloat3ToStream( temp.Specular );
		archiver->Space();
		archiver->WriteToStream( temp.Position );
		archiver->Space();
		archiver->WriteToStream( temp.LookAt );
		archiver->WriteToStream( '\n' );
	}

	for ( std::vector<CShadowPointLight>::iterator i = m_ShadowPointLights.begin(); i != m_ShadowPointLights.end(); ++i )
	{
		CShadowPointLight light = *i;
		PointLight temp = light.GetPointLight( 0 );
		archiver->WriteToStream('l');
		archiver->Space();
		archiver->WriteFloat3ToStream( temp.Ambient );
		archiver->Space();
		archiver->WriteFloat3ToStream( temp.Diffuse );
		archiver->Space();
		archiver->WriteToStream( temp.Att );
		archiver->Space();
		archiver->WriteToStream( temp.Range );
		archiver->Space();
		archiver->WriteFloat3ToStream( temp.Specular );
		archiver->Space();
		archiver->WriteToStream( temp.Position );
		archiver->Space();
		archiver->WriteToStream( XMFLOAT3( SHADOW_PL, SHADOW_PL, SHADOW_PL ) );
		archiver->WriteToStream( '\n' );
	}
}

// Loads a new lighting scene and all lights based off the lighting settings
bool CLightManager::LoadScene(CSceneLighting settings)
{
	for ( int i = 0; i < settings.pointLights.size(); ++i )
	{
		PointLight p = settings.pointLights.at( i );
		if ( p.LookAt.x == SHADOW_PL )
		{
			// Actually a shadow casting point light.
			CShadowPointLight spl;
			spl.Init( p );
			m_ShadowPointLights.push_back( spl );
			settings.pointLights.erase( settings.pointLights.begin() + i, settings.pointLights.begin() + i + 1 );
		}
	}
	mLightingInfo = settings;

	if(mIsSceneLoaded)
	{
		DestroySymbols();
	}
	bool result;
	
	BulletData dat;
	dat.SpecGrav = 0.0f;
	for(int i = 0; i < mLightingInfo.pointLights.size(); ++i)
	{
		XMFLOAT3 pos = mLightingInfo.pointLights.at(i).Position;
		CStaticObject* obj = new CStaticObject(D3DXVECTOR3(pos.x,pos.y,pos.z),D3DXVECTOR3(0,0,0),XMFLOAT3(0.2f,0.2f,0.2f),XMFLOAT3(1.0f,1.0f,1.0f),XMFLOAT3(0.0f,0.0f,0.0f));
		mLightSymbols.push_back( obj );
		result = obj->Initialize(DXUTGetD3D11Device(),"untitled.obj",eBASIC,dat);
		if(!result)
			return false;
	}
	for(int i = 0; i < m_ShadowPointLights.size(); ++i)
	{
		XMFLOAT3 pos = m_ShadowPointLights.at( i ).GetPointLight( 0 ).Position;
		CStaticObject* obj = new CStaticObject(D3DXVECTOR3(pos.x,pos.y,pos.z),D3DXVECTOR3(0,0,0),XMFLOAT3(0.2f,0.2f,0.2f),XMFLOAT3(1.0f,1.0f,1.0f),XMFLOAT3(0.0f,0.0f,0.0f));
		mLightSymbols.push_back( obj );
		result = obj->Initialize(DXUTGetD3D11Device(),"untitled.obj",eBASIC,dat);
		if(!result)
			return false;
	}

	DirectionalLight dirLight;
	dirLight.Ambient  = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	dirLight.Diffuse  = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	dirLight.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	dirLight.Direction = XMFLOAT3(0.2548f, -0.57735, 0.57757f);

	m_vOriginalLightDir = Vec( dirLight.Direction );
	
	mLightingInfo.dirLights.push_back( dirLight );

	mIsSceneLoaded = true;
	return true;
}

void CLightManager::Shutdown()
{
	DestroySymbols();
	mIsSceneLoaded = false;
}

void CLightManager::RenderLightSymbols(ID3D11DeviceContext* context, CEffectManager* effectManager,CameraInfo& cam)
{
	ExtraRenderingData erd;
	erd.pShadowMap = NULL;
	erd.ShadowTransform = XMMatrixIdentity();
	for(int i = 0; i < mLightSymbols.size(); ++i)
	{
		mLightSymbols.at(i)->Render( context, cam, effectManager, erd );
	}
}


void CLightManager::DestroySymbols(void)
{
	for(int i = 0; i < mLightSymbols.size(); ++i)
	{
		DestroyObject(mLightSymbols.at(i));
	}
	mLightSymbols.clear();
}


void CLightManager::DestroyResources()
{
	for(int i = 0; i < mLightSymbols.size(); ++i)
		ReleaseResources(mLightSymbols.at(i));
}

void CLightManager::Update(GameStates gameState, float dt)
{
	if(gameState == eBuildingLevel)
	{
		
		if(GetAsyncKeyState('Z') & 0x8000 && !mWindow.mIsWindow)
		{
			XMFLOAT3 pos, att, diff, spec, ambient, lookAt;
			float range;
			if ( mSelectedLight < mLightingInfo.pointLights.size() )
			{
				PointLight p = mLightingInfo.pointLights.at( mSelectedLight );
				range = p.Range;
				pos = p.Position;
				att = p.Att;
				diff = XMFLOAT3( p.Diffuse.x, p.Diffuse.y, p.Diffuse.z );
				ambient = XMFLOAT3( p.Ambient.x, p.Ambient.y, p.Ambient.z );
				spec = XMFLOAT3( p.Specular.x, p.Specular.y, p.Specular.z );
				lookAt = p.LookAt;
			}
			else
			{
				int shadowIndex = mSelectedLight - mLightingInfo.pointLights.size();
				PointLight p = m_ShadowPointLights.at( shadowIndex ).GetPointLight( 0 );
				range = p.Range;
				pos = p.Position;
				att = p.Att;
				ambient = XMFLOAT3( p.Ambient.x, p.Ambient.y, p.Ambient.z );
				spec = XMFLOAT3( p.Specular.x, p.Specular.y, p.Specular.z );
				diff = XMFLOAT3( p.Diffuse.x, p.Diffuse.y, p.Diffuse.z );
				lookAt = XMFLOAT3( SHADOW_PL, SHADOW_PL, SHADOW_PL );
			}
			

			if ( mWindow.RunWindow( IDD_DIALOG_LIGHT_PROPERTIES, mhInstance, mhWnd, pos.x, pos.y, pos.z, range, att, diff, spec, ambient, lookAt ) == FALSE )
			{
				MessageBoxA(0,"Unhandeled exception!\n Window creation failed!\nYou Suck!","Fatal Error",MB_OK);
			}
		}
		if ( GetAsyncKeyState('X') & 0x8000 )
		{
			RemovePointLight( mSelectedLight );
		}

		if(mWindow.WasExitButtonPressed())
		{
			XMFLOAT3 att;
			XMFLOAT4 diff;
			XMFLOAT3 lookAt;
			XMFLOAT4 ambient;
			XMFLOAT4 spec;
			diff.w = 1.0f;
			spec.w = 1.0f;
			ambient.w = 1.0f;
			float* newData = mWindow.GetSpecifiedPosAsFloat();

			float x		= newData[0];
			float y		= newData[1];
			float z		= newData[2];

			att.x		= newData[3];
			att.y		= newData[4];
			att.z		= newData[5];

			diff.x		= newData[6];
			diff.y		= newData[7];
			diff.z		= newData[8];

			lookAt.x	= newData[9];
			lookAt.y	= newData[10];
			lookAt.z	= newData[11];

			spec.x		= newData[12];
			spec.y		= newData[13];
			spec.z		= newData[14];

			ambient.x	= newData[15];
			ambient.y	= newData[16];
			ambient.z	= newData[17];

			if ( mSelectedLight < mLightingInfo.pointLights.size() )
			{
				mLightingInfo.pointLights.at( mSelectedLight ).Position = XMFLOAT3( x, y, z );
				mLightingInfo.pointLights.at( mSelectedLight ).Range = mWindow.GetRangeTextbox();
				mLightingInfo.pointLights.at( mSelectedLight ).Att = att;
				mLightingInfo.pointLights.at( mSelectedLight ).Diffuse = diff;
				mLightingInfo.pointLights.at( mSelectedLight ).Ambient = ambient;
				mLightingInfo.pointLights.at( mSelectedLight ).Specular = spec;
				mLightingInfo.pointLights.at( mSelectedLight ).LookAt = lookAt;
			}
			else
			{
				int shadowIndex = mSelectedLight - mLightingInfo.pointLights.size();
				m_ShadowPointLights.at( shadowIndex ).SetInfo( diff, att, spec, ambient, mWindow.GetRangeTextbox() );
				m_ShadowPointLights.at( shadowIndex ).SetPosition( Vec( x, y, z ) );
			}
			

			mLightSymbols.at( mSelectedLight )->SetPos( x, y, z );
			mWindow.DestroyWindow();
		}
		CHashedString message;
		while(mEventListener.HasPendingMessages())
		{
			message = mEventListener.GetRecievedMessage();
			const float moveSpeed = 0.05f;
			const float rotateSpeed = 0.05f;

			if ( mEventListener.GetSelectedMgr() == eManager::eLightMgr )
			{
#pragma region ObjectModifiers
				if ( message == "c1EditorMoveLeft" )
				{
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).Position.x -= moveSpeed;
					}
					else
					{
						int shadowIndex = mSelectedLight - mLightingInfo.pointLights.size();
						m_ShadowPointLights.at( shadowIndex ).Move( Vec( -moveSpeed, 0, 0 ) );
					}

					mLightSymbols.at( mSelectedLight )->MoveX( -moveSpeed );
				}
				else if(message == "c1EditorMoveRight")
				{
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).Position.x += moveSpeed;
					}
					else
					{
						int shadowIndex = mSelectedLight - mLightingInfo.pointLights.size();
						m_ShadowPointLights.at( shadowIndex ).Move( Vec( moveSpeed, 0, 0 ) );
					}
					mLightSymbols.at( mSelectedLight )->MoveX( moveSpeed );
				}
				else if(message == "c1EditorMoveForward")
				{
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).Position.z -= moveSpeed;
					}
					else
					{
						int shadowIndex = mSelectedLight - mLightingInfo.pointLights.size();
						m_ShadowPointLights.at( shadowIndex ).Move( Vec( 0, 0, moveSpeed ) );
					}
					mLightSymbols.at( mSelectedLight )->MoveZ( moveSpeed );
				}
				else if(message == "c1EditorMoveBackward")
				{
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).Position.z -= moveSpeed;
					}
					else
					{
						int shadowIndex = mSelectedLight - mLightingInfo.pointLights.size();
						m_ShadowPointLights.at( shadowIndex ).Move( Vec( 0, 0, -moveSpeed ) );
					}
					mLightSymbols.at( mSelectedLight )->MoveZ( -moveSpeed );
				}
				else if(message == "c1EditorMoveUp")
				{
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).Position.y += moveSpeed;
					}
					else
					{
						int shadowIndex = mSelectedLight - mLightingInfo.pointLights.size();
						m_ShadowPointLights.at( shadowIndex ).Move( Vec( 0, moveSpeed, 0 ) );
					}
					mLightSymbols.at( mSelectedLight )->MoveY( moveSpeed );
				}
				else if(message == "c1EditorMoveDown")
				{
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).Position.y -= moveSpeed;
					}
					else
					{
						int shadowIndex = mSelectedLight - mLightingInfo.pointLights.size();
						m_ShadowPointLights.at( shadowIndex ).Move( Vec( 0, -moveSpeed, 0 ) );
					}
					mLightSymbols.at( mSelectedLight )->MoveY( -moveSpeed );
				}
				else if(message == "c1EditorRotateXPos")
				{
					
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).LookAt.x += moveSpeed;
					}
					else
					{
						//m_ShadowPointLights.at( mSelectedLight ).Move( Vec( -moveSpeed, 0, 0 ) );
					}
				}
				else if(message == "c1EditorRotateXNeg")
				{
					
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).LookAt.x -= moveSpeed;
					}
					else
					{
						//m_ShadowPointLights.at( mSelectedLight ).Move( Vec( -moveSpeed, 0, 0 ) );
					}
				}
				else if(message == "c1EditorRotateYPos")
				{
					
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).LookAt.y += moveSpeed;
					}
					else
					{
						//m_ShadowPointLights.at( mSelectedLight ).Move( Vec( -moveSpeed, 0, 0 ) );
					}
				}
				else if(message == "c1EditorRotateYNeg")
				{
					
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).LookAt.y -= moveSpeed;
					}
					else
					{
						//m_ShadowPointLights.at( mSelectedLight ).Move( Vec( -moveSpeed, 0, 0 ) );
					}
				}
				else if(message == "c1EditorScalePos")
				{
					// This is now our positive z-axis rotation.
					
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).LookAt.z += moveSpeed;
					}
					else
					{
						//m_ShadowPointLights.at( mSelectedLight ).Move( Vec( -moveSpeed, 0, 0 ) );
					}
				}
				else if(message == "c1EditorScaleNeg")
				{
					// This is now our negative z-axis rotation.
					
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).LookAt.z -= moveSpeed;
					}
					else
					{
						//m_ShadowPointLights.at( mSelectedLight ).Move( Vec( -moveSpeed, 0, 0 ) );
					}
				}
				else if(message == "c1EditorTexScalePos")
				{
					// This is now increasing the range.
					
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).Range += moveSpeed;
					}
					else
					{
						int shadowIndex = mSelectedLight - mLightingInfo.pointLights.size();
						m_ShadowPointLights.at( shadowIndex ).AddRange( moveSpeed );
					}
				}
				else if(message == "c1EditorTexScaleNeg")
				{
					// This is now decreasing the range.
					
					if ( mSelectedLight < mLightingInfo.pointLights.size() )
					{
						mLightingInfo.pointLights.at( mSelectedLight ).Range -= moveSpeed;
					}
					else
					{
						int shadowIndex = mSelectedLight - mLightingInfo.pointLights.size();
						m_ShadowPointLights.at( shadowIndex ).AddRange( -moveSpeed );
					}
				}
#pragma endregion
			}

			if( message == "c1EditorAddSelectedLight" )
			{
				if(mSelectedLight + 1 < mLightingInfo.pointLights.size() +  m_ShadowPointLights.size() )
				{
					++mSelectedLight;
					mEventListener.SetSelectedLight(mSelectedLight);
				}
			}
			else if( message == "c1EditorSubtractSelectedLight")
			{
				if(mSelectedLight - 1 >= 0)
				{
					--mSelectedLight;
					mEventListener.SetSelectedLight(mSelectedLight);
				}
			}
			else if( message == "c1AddPointLight")
			{
				AddPointLight();
			}
		}
	}

	// Uncomment this to rotate the light.
	m_fLightRotationAngle += 0.1f * dt;

	XMMATRIX R = XMMatrixRotationY(m_fLightRotationAngle);

	XMVECTOR lightDir = XMLoadFloat3( &m_vOriginalLightDir.ToFloat3() );
	lightDir = XMVector3TransformNormal( lightDir, R );
	XMStoreFloat3( &mLightingInfo.dirLights.at(0).Direction, lightDir );

	UpdateShadowTransform();
}

void CLightManager::AddPointLight()
{
	BulletData dat;
	dat.SpecGrav = 0.0f;
	if ( GetNumberOfPointLights() != MAX_NUMBER_OF_POINT_LIGHTS )
	{
		WCF::ModalWindow addPointLightWindow = WCF::ModalWindow();
		INT_PTR dialogResult = addPointLightWindow.RunWindow( DXUTGetHINSTANCE(), DXUTGetHWND(), MAKEINTRESOURCE( IDD_DIALOG_ADD_LIGHT ) );
		if ( dialogResult == IDC_BUTTON_AL_ADD_PL )
		{
			PointLight pointLight;
			mLightingInfo.pointLights.push_back(pointLight);
		}
		else if ( dialogResult == IDC_BUTTON_AL_ADD_SCPL )
		{
			PointLight pointLight;
			CShadowPointLight spl;
			spl.Init( pointLight );
			m_ShadowPointLights.push_back( spl );
		}
		else
			return;

		
		CStaticObject* model = new CStaticObject( D3DXVECTOR3(0.0f,5.0f,0.0f), D3DXVECTOR3(0.0f,0.0f,0.0f), XMFLOAT3(0.2f,0.2f,0.2f), XMFLOAT3(1.0f,1.0f,1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) );
		bool result = model->Initialize(DXUTGetD3D11Device(),"untitled.obj",eBASIC,dat);
		mLightSymbols.push_back(model);
		mSelectedLight = 0;
	}
}

void CLightManager::RemovePointLight(int index)
{
	if ( ( !mLightingInfo.pointLights.empty() || !m_ShadowPointLights.empty() ) && index > -1 )
	{
		if ( index < mLightingInfo.pointLights.size() )
		{
			std::vector<PointLight>::iterator beginErase = mLightingInfo.pointLights.begin() + index;
			// Remove the light from the info list which is used to actually light the scene.
			mLightingInfo.pointLights.erase( beginErase, beginErase + 1 );
			
			mSelectedLight = -1;
		}
		else
		{
			int shadowPLIndex = index - mLightingInfo.pointLights.size();
			std::vector<CShadowPointLight>::iterator beginErase = m_ShadowPointLights.begin() + shadowPLIndex;
			m_ShadowPointLights.erase( beginErase, beginErase + 1 );
		}

		// Then remove from the symbols list which is used during builder mode.
		ReleaseResources( mLightSymbols.at( index ) );
		DestroyObject( mLightSymbols.at( index ) );
		mLightSymbols.erase( mLightSymbols.begin() + index, (mLightSymbols.begin() + index ) + 1 );
	}
}

XMMATRIX CLightManager::GetLightView()
{
	return XMLoadFloat4x4( &m_4x4LightView );
}

XMMATRIX CLightManager::GetLightProj()
{
	return XMLoadFloat4x4( &m_4x4LightProj );
}

XMMATRIX CLightManager::GetShadowTransform()
{
	return XMLoadFloat4x4( &m_4x4ShadowTransform );
}

void CLightManager::UpdateShadowTransform()
{
	DirectionalLight mainDirLight = mLightingInfo.dirLights.at( 0 );

	//// Transform bounding sphere to light space.
	//XMFLOAT3 sphereCenterLS;
	//XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

	//// Ortho frustum in light space encloses scene.
	//float l = sphereCenterLS.x - sceneRadius;
	//float b = sphereCenterLS.y - sceneRadius;
	//float n = sphereCenterLS.z - sceneRadius;
	//float r = sphereCenterLS.x + sceneRadius;
	//float t = sphereCenterLS.y + sceneRadius;
	//float f = sphereCenterLS.z + sceneRadius;
	//XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
	// Generate this version fo the projection matrix for point lights. 

	float fieldOfView = XM_PI / 2.0f;
	float screenAspect = 1.0f;
	const float zFar = 2500.0f;
	const float zNear = 0.1f;
	XMMATRIX P = XMMatrixPerspectiveFovLH( fieldOfView, screenAspect, zNear, zFar );

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMStoreFloat4x4(&m_4x4LightProj, P);
}


XMMATRIX CLightManager::ConstructPointLightView( int index, Vec& lookAt )
{
	PointLight pointLight = mLightingInfo.pointLights.at( index );
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR lightPos = XMLoadFloat3( &pointLight.Position );
	XMVECTOR targetPos = Vec(0.0f, 0.0f, 0.0f).ToVector();

	XMMATRIX V = XMMatrixLookAtLH( lightPos, targetPos, up );
	return V;
}

XMMATRIX CLightManager::ConstructShadowTransform( CXMMATRIX view, CXMMATRIX proj )
{
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = view * proj * T;
	return S;
}


void CLightManager::GeneratePointLightView( int index )
{
	PointLight pointLight;
	if ( index < mLightingInfo.pointLights.size() )
		pointLight = mLightingInfo.pointLights.at( index );
	else
	{
		int shadowPointLightIndex = index - mLightingInfo.pointLights.size();
		// Since each shadow point light contains 6 point lights.
		int accessIndex = shadowPointLightIndex / 6;
		int indexOfPointLight = shadowPointLightIndex - ( accessIndex * 6 );
		pointLight = m_ShadowPointLights.at( accessIndex ).GetPointLight( indexOfPointLight );
	}

	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMVECTOR lookAt = XMLoadFloat3( &pointLight.LookAt );

	XMVECTOR targetPos = lookAt;
	XMVECTOR lightPos = XMLoadFloat3( &pointLight.Position );
	

	XMMATRIX V = XMMatrixLookAtLH( lightPos, targetPos, up );

	XMMATRIX P = XMLoadFloat4x4( &m_4x4LightProj );

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = V * P * T;

	XMStoreFloat4x4( &m_4x4LightView, V );
	XMStoreFloat4x4( &m_4x4ShadowTransform, S );
}

int CLightManager::GetNumberOfPointLights()
{
	// Don't count the point lights that don't cast shadows.
	int noShadowPlCount = 0;
	for ( int i = 0; i < mLightingInfo.pointLights.size(); ++i )
	{
		PointLight pl = mLightingInfo.pointLights.at( i );
		bool isNoShadowPl = IS_NO_SHADOW_PL( pl );
		if  ( isNoShadowPl )
			++noShadowPlCount;
	}
	return ( mLightingInfo.pointLights.size() +  ( m_ShadowPointLights.size() * 6 ) ) - noShadowPlCount;
}

PointLight* CLightManager::CreatePointLight( PointLight pointLight )
{
	mLightingInfo.pointLights.push_back( pointLight );
	PointLight* p = &mLightingInfo.pointLights.at( mLightingInfo.pointLights.size() - 1 );

	return p;
}

void CLightManager::DestroyGameAssets()
{
	// Everything for light manager is a game asset so just completely shut down.
	DestroyResources();
	Shutdown();

	m_ShadowPointLights.clear();
	m_ShadowPointLights.resize(0);
}
