//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "ObjectManager.h"
#include <algorithm>
#include "LightManager.h"

CObjectManager::CObjectManager(CEventManager* eventManager, HINSTANCE hInstance,CGameObjMgr* mgr) 
	: mEventListener(eInputEventType), mSelectedObject(0), mWindow(), mOPPWindow()
{
	mhInstance = hInstance;
	m_ptObjMgr = mgr;
	eventManager->RegisterListener(&mEventListener);
}

CObjectManager::~CObjectManager()
{

}

bool CObjectManager::InitResources(ID3D11Device* device)
{
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc,sizeof(noCullDesc));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	if(FAILED(device->CreateRasterizerState(&noCullDesc,&mNoCullRS)))
		return false;
	
	
	return true;
}

void CObjectManager::Shutdown()
{
	
	for(int i = 0; i<mObjectList.size();++i)
	{
		DestroyObject(mObjectList.at(i));
	}
	for(int i = 0; i < mSimpleObjectList.size(); ++i)
	{
		DestroyObject(mSimpleObjectList.at(i));
	}
}

void CObjectManager::DestroyResources()
{
	
	ReleaseCOM(mNoCullRS);
	for(int i = 0; i < mObjectList.size();++i)
	{
		ReleaseResources(mObjectList.at(i));
	}
	for(int i = 0; i < mSimpleObjectList.size(); ++i)
	{
		ReleaseResources(mSimpleObjectList.at(i));
	}
}

void CObjectManager::Render(ID3D11DeviceContext* context,CameraInfo& cam,const CEffectManager* effectManager, ExtraRenderingData& erd)
{
	//Set all effect things necissary to render
	//Set no cull so the entire objects render
	context->RSSetState( mNoCullRS );
	SRenderSettings::SetDefaultDSS();
	SRenderSettings::SetDefaultBS();
	for(int i = 0; i < mObjectList.size(); ++i)
	{
		mObjectList.at(i)->Render( context, cam, effectManager, erd );
	}
	
	context->RSSetState(0);
}

void CObjectManager::RenderShadowMap(ID3D11DeviceContext* context,CameraInfo& cam,const CEffectManager* effectManager, CLightManager* pLightMgr)
{
	for(int i = 0; i < mObjectList.size(); ++i)
	{
		mObjectList.at(i)->RenderShadowMap( context, cam, effectManager, pLightMgr );
	}
	context->RSSetState( 0 );
}


bool CObjectManager::LoadScene(ID3D11Device* device,vector<ObjectInfo> sceneInfo)
{
	DestroyObjects();

	ObjectInfo objectInfo;
	for(int i = 0; i<sceneInfo.size(); ++i)
	{
		objectInfo = sceneInfo.at(i);
		CStaticObject* object = new CStaticObject(D3DXVECTOR3(objectInfo.x,objectInfo.y,objectInfo.z),D3DXVECTOR3(objectInfo.rx,objectInfo.ry,objectInfo.rz),XMFLOAT3(objectInfo.s, objectInfo.sy, objectInfo.sz),
			XMFLOAT3(objectInfo.tsx,objectInfo.tsy,objectInfo.tsz), XMFLOAT3(objectInfo.ttx,objectInfo.tty,objectInfo.ttz));
		if(!object->Initialize(device,objectInfo.ModelName,objectInfo.RenderTech,objectInfo.BulletData,objectInfo.RenderTechDat))
		{
			FatalError("Couldn't load the static object!\nbool CObjectManager::LoadScene(ID3D11Device* device,vector<ObjectInfo> sceneInfo)");
			return false;
		}
		
		m_ptObjMgr->AddObject(object);

		mObjectList.push_back(object);
		
	}

	//std::sort( mObjectList.begin(), mObjectList.end() );

	return true;
}

void CObjectManager::Update(GameStates gameState, bool flash)
{	
	const float moveSpeed = 0.05f;
	const float rotateSpeed = 0.2f;
	const float scaleSpeed = 0.08f;

	if(gameState == eBuildingLevel)
	{
		CHashedString message;
		while(mEventListener.HasPendingMessages())
		{
			message = mEventListener.GetRecievedMessage();

			if ( mEventListener.GetSelectedMgr() == eManager::eObjectMgr )
			{
#pragma region ObjectPositionModifiers
			if(message == "c1EditorMoveLeft")
			{
				mObjectList.at(mSelectedObject)->MoveX(-moveSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorMoveRight")
			{
				mObjectList.at(mSelectedObject)->MoveX(moveSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorMoveForward")
			{
				mObjectList.at(mSelectedObject)->MoveZ(moveSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorMoveBackward")
			{
				mObjectList.at(mSelectedObject)->MoveZ(-moveSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorMoveUp")
			{
				mObjectList.at(mSelectedObject)->MoveY(moveSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorMoveDown")
			{
				mObjectList.at(mSelectedObject)->MoveY(-moveSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorRotateXPos")
			{
				mObjectList.at(mSelectedObject)->RotateX(rotateSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorRotateXNeg")
			{
				mObjectList.at(mSelectedObject)->RotateX(-rotateSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorRotateYPos")
			{
				mObjectList.at(mSelectedObject)->RotateY(rotateSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorRotateYNeg")
			{
				mObjectList.at(mSelectedObject)->RotateY(-rotateSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorScalePos")
			{
				mObjectList.at(mSelectedObject)->Scale(scaleSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorScaleNeg")
			{
				mObjectList.at(mSelectedObject)->Scale(-scaleSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorTexScalePos")
			{
				mObjectList.at(mSelectedObject)->TexScale(scaleSpeed);
				PhysicsSynced( false );
			}
			else if(message == "c1EditorTexScaleNeg")
			{
				mObjectList.at(mSelectedObject)->TexScale(-scaleSpeed);
			}
#pragma endregion
			}

			if(message == "c1EditorAddSelectedObject")
			{
				if(mSelectedObject + 1 < mObjectList.size())
				{
					mObjectList.at( mSelectedObject )->SetFlashing( 0 );
					++mSelectedObject;
					mEventListener.RemoveAllOfMessage( message );
				}
			}
			else if(message == "c1EditorSubtractSelectedObject")
			{
				if(mSelectedObject > 0)
				{
					mObjectList.at( mSelectedObject )->SetFlashing( 0 );
					--mSelectedObject;
					mEventListener.RemoveAllOfMessage( message );
				}
			}
			else if(message == "c1SelectedObjectDeleted")
			{
				mEventListener.RemoveAllOfMessage(CHashedString("c1SelectedObjectDeleted"));
				DeleteObject(mSelectedObject);
			}
			else if(message == "c1ObjectAdded")
			{
				mEventListener.RemoveAllOfMessage(CHashedString("c1ObjectAdded"));
				
				if(!AddObject(mEventListener.GetCreatedObjectBuffer(),DXUTGetD3D11Device()))
					MessageBoxA(0,"Couldn't load the object you wanted!\n(Chances are it doesn't exist","Error", MB_OK);
			}
			
			else if(message == "c1OpenObjectProperties")
			{
				mEventListener.RemoveAllOfMessage( message );
				StaticObjectData data = mObjectList.at( mSelectedObject )->GetPositionData();					
				RenderTechData rtd = mObjectList.at( mSelectedObject )->GetRenderData();
				if(mWindow.RunWindow( mhInstance, mhWnd, data, mObjectList.at( mSelectedObject )->GetRenderTech(), rtd ) == FALSE)
					FatalError("Couldn't load the Object Prop Dialog!");
			}

			else if(message == "c1CreateAddObjectDlg")
			{
				
			}

			else if( message == "c1CreateOPPWindow" )
			{
				mEventListener.RemoveAllOfMessage( message );
				if ( !mOPPWindow.mIsWindow )
					CreateOPPWindow();
			}

			else if ( message == "c1UpdatePhysics" )
			{
				// This probaly shouldn't be handeled here or under the
				// 1 namespace but whatever.
				//TODO:
				// Fix the above.
				m_ptObjMgr->ResyncPhysics();
			}
		}
		
		UpdateAllWindows();
		if ( flash )
			mObjectList.at( mSelectedObject )->SetFlashing( GREEN_FLASH_COLOR );
	}

	mEventListener.SetSelectedObject(mSelectedObject);

		
}

bool CObjectManager::Initialize(CEventManager* eventManager,HWND hwnd)
{
	eventManager->RegisterListener(&mEventListener);
	eventManager->RegisterObjMgr( this );
	mhWnd = hwnd;

	return true;
}

void CObjectManager::Serilize(CDataArchiver* dataArchiver)
{
	for(int i = 0; i < mObjectList.size(); ++i)
	{
		mObjectList.at(i)->Serilize(dataArchiver);
	}
}

// Destroys and releases all current objects in the scene
void CObjectManager::DestroyObjects(void)
{
	for(int i = 0; i < mObjectList.size();++i)
	{
		ReleaseResources(mObjectList.at(i));
	}
	for(int i = 0; i<mObjectList.size();++i)
	{
		DestroyObject(mObjectList.at(i));
	}
	mObjectList.clear();
	mObjectList.resize(0);
}


void CObjectManager::DeleteObject(int index)
{
	if(mSelectedObject != 0)
		--mSelectedObject;
	else
		++mSelectedObject;
	ReleaseResources(mObjectList.at(index));
	DestroyObject(mObjectList.at(index));
	mObjectList.erase(mObjectList.begin() + index);
}


bool CObjectManager::AddObject(const char* filename, ID3D11Device* device)
{
	CStaticObject* object = new CStaticObject();
	BulletData dat;
	dat.Mat = PhysMat_Normal;
	dat.SpecGrav = 4.0f;
	if ( !object->Initialize( device, filename, eBASIC, dat ) )
	{
		DestroyObject(object);
		return false;
	}
	mObjectList.push_back(object);
	m_ptObjMgr->AddObject( object );

	///CHANGE THIS HERE
	//m_ptObjMgr->OnStaticCtrlAdded(mObjectList.at(mObjectList.size() - 1)->GetBoundingBox());


	return true;
}

bool CObjectManager::AddObject(const char* filename, ID3D11Device* device,PositionData data,float tx,float ty,float tz)
{
	BulletData dat;
	dat.SpecGrav = 4.0f;
	dat.Mat = PhysMat_Normal;
	CStaticObject* object = new CStaticObject(D3DXVECTOR3(data.x,data.y,data.z), D3DXVECTOR3(data.rx,data.ry,data.rz), XMFLOAT3( data.s, data.s, data.s ), XMFLOAT3( tx, ty, tz ),XMFLOAT3( 0.0f, 0.0f, 0.0f ) );
	if(!object->Initialize(device,filename,eBASIC,dat))
	{
		DestroyObject(object);
		return false;
	}
	mObjectList.push_back(object);
	m_ptObjMgr->AddObject( object );
	

	return true;
}


void CObjectManager::CreateOPPWindow()
{
	ObjectPhysicsPropData data = mObjectList.at( mSelectedObject )->GetPhysicsProperties();

	Vec grav = m_ptObjMgr->GetGlobalGravity();
	data.GravityX = grav.X();
	data.GravityY = grav.Y();
	data.GravityZ = grav.Z();

	if ( mOPPWindow.RunWindow( mhInstance, mhWnd, data ) == FALSE )
		FatalError( "Couldn't create the opp window!\nvoid CObjectManager::CreateOPPWindow()" );
}

void CObjectManager::UpdateAllWindows()
{
	if(mWindow.WasExitButtonPressed())
	{
		float* numbers;
		numbers = mWindow.GetObjectData();
		StaticObjectData data(numbers[0],numbers[1],numbers[2],
			numbers[3],numbers[4],numbers[5],
			numbers[6],numbers[7],numbers[8],
			numbers[9],numbers[10],numbers[11],
			numbers[12],numbers[13],numbers[14],
			numbers[15]);
		mObjectList.at(mSelectedObject)->SetRenderTech(mWindow.GetRenderTech(mObjectList.at(mSelectedObject)->GetRenderTech()));
		mObjectList.at(mSelectedObject)->SetPositionData(data);
		mObjectList.at( mSelectedObject )->SetRenderData( mWindow.GetRenderTechData() );
		mWindow.DestroyWindow();
	}

	if ( mOPPWindow.WasExitButtonPressed() )
	{
		ObjectPhysicsPropData data = mOPPWindow.GetWindowData( );
		BulletData bDat;
		bDat.SpecGrav = data.SpecGrav;
		bDat.Mat = (PhysicsMaterial)data.Material;
		
		// Get the ( possibly ) updated gravity.
		Vec newGrav;
		newGrav.SetX( data.GravityX );
		newGrav.SetY( data.GravityY );
		newGrav.SetZ( data.GravityZ );
		m_ptObjMgr->SetGlobalGravity( newGrav );

		mObjectList.at( mSelectedObject )->SetAllPhysicsData( bDat );
		mOPPWindow.DestroyWindow();
	}
}

void CObjectManager::RenderRefractableObjects( CameraInfo& cam, CEffectManager* effectManager )
{
	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();
	context->RSSetState(mNoCullRS);
	SRenderSettings::SetDefaultDSS();
	SRenderSettings::SetDefaultBS();

	// Iterate through the list testing for refractiveness.
	for ( std::vector<CStaticObject*>::iterator i = mObjectList.begin(); i != mObjectList.end(); ++i )
	{
		if ( (*i)->GetRenderData().Refractable )
		{
			(*i)->RenderRefraction( context, cam, effectManager );
		}
	}

	context->RSSetState(0);
}

void CObjectManager::RenderReflectableObjects( CameraInfo& cam, CEffectManager* effectManager, ExtraRenderingData& rtd, float waterHeight )
{
	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();
	context->RSSetState(mNoCullRS);
	SRenderSettings::SetDefaultDSS();
	SRenderSettings::SetDefaultBS();

	for ( int i = 1; i < 7; ++i )
	{
		mObjectList.at( i )->Render( context, cam, effectManager, rtd );
	}

	context->RSSetState(0);
}


void CObjectManager::OnObjectClicked( ObjectId idOfObject )
{
	//DebugWriter::WriteToDebugFile( "Used deprecated function!" );
}

void CObjectManager::OnSelectedObjectChanged( int newSelectedObject )
{
	mObjectList.at( mSelectedObject )->SetFlashing( 0 );
	mSelectedObject = newSelectedObject;
}

CStaticObject* CObjectManager::GetObjectByIndex( int index )
{
	return mObjectList.at( index );
}

ObjectId CObjectManager::InsertObject( CStaticObject* object )
{
	m_ptObjMgr->AddObject(object);
	mObjectList.push_back(object);

	return mObjectList.size() - 1;
}

void CObjectManager::DestroyGameAssets()
{
	DestroyObjects();
}
