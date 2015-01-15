//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "AnimObjectMgr.h"
#include "RenderEffects.h"


CAnimObjectMgr::CAnimObjectMgr()
	: CManager(DXUTGetHINSTANCE(),DXUTGetHWND(),eAnimEventType,new WCF::AnimPropDialog()), 
	m_TextureMgr()
{
	m_iSelectedObject = 0;
	m_iIndex = -1;
}


CAnimObjectMgr::~CAnimObjectMgr(void)
{
}


BOOL CAnimObjectMgr::CreateDlgWindow(void)
{
	WCF::AnimPropDialog* window = (WCF::AnimPropDialog*)mWindow;
	WCF::AnimPropDialog::AnimPropData data = mObjectList.at(m_iSelectedObject)->GetAllData();
	data.Selected = m_iSelectedObject;

	return window->RunWindow(mhInstance,mhWnd,data);
}


bool CAnimObjectMgr::Init(CEventManager* eventManager, CHashedString messageToWatch,CGameObjMgr* ctrlMgr)
{
	CManager::Init(eventManager,messageToWatch);
	m_ptObjCtrlMgr = ctrlMgr;
	eventManager->RegisterAnimObjMgr(this);

	return true;
}


bool CAnimObjectMgr::InitializeResources(ID3D11Device* device)
{
	CManager::InitializeResources(device);

	m_TextureMgr.Init(device);

	mEffect = new CSkinEffect( device, L"Skinned.fx" );
	assert( mEffect );

	const D3D11_INPUT_ELEMENT_DESC desc[] = 
	{
		{"POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"WEIGHTS",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONEINDICES",  0, DXGI_FORMAT_R8G8B8A8_UINT,   0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	D3DX11_PASS_DESC passDesc;
	mEffect->LightTexSkin->GetPassByIndex(0)->GetDesc(&passDesc);
	HRESULT hr = device->CreateInputLayout(desc,ARRAYSIZE(desc),passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&mInputLayout);
	Checkh( hr );

	return true;
}


void CAnimObjectMgr::Shutdown(void)
{
	m_ptObjCtrlMgr = 0;
	CManager::Shutdown();
	for(int i = 0; i < mObjectList.size(); ++i)
	{
		DestroyObject( mObjectList.at( i ) );
	}
	m_ObjectsToRender.clear();
}


void CAnimObjectMgr::DestroyResources(void)
{
	CManager::DestroyResources();

	SafeDelete( mEffect );
	ReleaseCOM( mInputLayout );

	for(int i = 0; i < mObjectList.size(); ++i)
	{
		mObjectList.at(i)->DestroyResources();
	}
}


void CAnimObjectMgr::Update(double time, float elapsedTime)
{
	CManager::Update(elapsedTime);
	for(int i = 0; i < mObjectList.size(); ++i)
	{
		mObjectList.at(i)->Update(elapsedTime);
	}
}


void CAnimObjectMgr::Serilize(CDataArchiver* archiver)
{
	// Implement
}


void CAnimObjectMgr::OnDialogQuit(void)
{
	WCF::AnimPropDialog* dialog = (WCF::AnimPropDialog*)mWindow;
	WCF::AnimPropDialog::AnimPropData data = dialog->GetWindowData();
	
	mObjectList.at(m_iSelectedObject)->SetAllData(data);
	m_iSelectedObject = data.Selected;
}


void CAnimObjectMgr::OnResized(const DXGI_SURFACE_DESC* sd, ID3D11Device* device)
{
}


void CAnimObjectMgr::Render(CameraInfo& cam)
{
	
	SRenderSettings::ResetAllStates();
	for(int i = 0; i < mObjectList.size(); ++i)
	{
		DXUTGetD3D11DeviceContext()->IASetInputLayout( mInputLayout );
		mObjectList.at( i )->Render( cam, mEffect );
	}
	/*if ( !m_ObjectsToRender.empty() )
	{
		SRenderSettings::SetNoCull(  );
		for ( std::vector<CStaticObject*>::iterator i = m_ObjectsToRender.begin(); i != m_ObjectsToRender.end(); ++i )
		{
			(*i)->Render( DXUTGetD3D11DeviceContext(), cam, effectManager );
		}
	}*/
}

CSkinEffect* CAnimObjectMgr::GetEffect()
{
	return mEffect;
}

void CAnimObjectMgr::RenderShadowMap( CLightManager* pLightMgr, CameraInfo& cam )
{
	
	for ( AnimatedObjectList::iterator i = mObjectList.begin(); i != mObjectList.end(); ++i )
	{
		DXUTGetD3D11DeviceContext()->IASetInputLayout( mInputLayout );
		(*i)->RenderShadowMap( pLightMgr, cam, mEffect );
	}
}


bool CAnimObjectMgr::AddObject(PositionData data,const char* filename,const wchar_t* textureDir,char* clipName)
{
	CAnimatedObject* object = new CAnimatedObject();
	float scY,scZ;
	scY = scZ = 0.0f;
	if(data.sy != 0.0f)
	{
		scY = data.sy;
		scZ = data.sz;
	}
	if(!object->Init(data))
		return false;
	if(!object->InitResources(DXUTGetD3D11Device(),filename,textureDir,clipName,m_TextureMgr))
		return false;

	mObjectList.push_back(object);


	//So the object actually appears.
	mObjectList.at(m_iSelectedObject)->Update(-1);


	m_ptObjCtrlMgr->AddObject(object);


	return true;
}


bool CAnimObjectMgr::AddObject(PositionData data,const char* filename,const wchar_t* textureDir,char* clipName,CPlayer* callback, BulletData db)
{
	CAnimatedObject* object = new CAnimatedObject();

	// Set all of the physics data.
	object->SetAllPhysicsData( db );

	float scY,scZ;
	scY = scZ = 0.0f;
	if(data.sy != 0.0f)
	{
		scY = data.sy;
		scZ = data.sz;
	}
	if(!object->Init(data))
		return false;
	if(!object->InitResources(DXUTGetD3D11Device(),filename,textureDir,clipName,m_TextureMgr))
		return false;

	mObjectList.push_back(object);


	// So the object actually appears.
	// The negitive one as a parameter just barely updates the object.
	mObjectList.at(m_iSelectedObject)->Update(-1);


	m_ptObjCtrlMgr->AddObject(object);

	// Call that callback
	callback->OnObjectCreated( object->GetId() );
	
	return true;
}


void CAnimObjectMgr::ListenToMessages(char* message)
{
	if(message == "AddObject")
	{
		AddObjectDlg();
	}
	else if(message == "DeleteObject")
	{
		DeleteSelectedObject();
	}
}

void CAnimObjectMgr::DeleteSelectedObject(void)
{
	if(m_iSelectedObject == 0)
	{
		DisplayError("Sorry currently you can't delete the root object!\nTry changing it instead.");
		return;
	}
	
	mObjectList.at(m_iSelectedObject)->DestroyResources();
	mObjectList.at(m_iSelectedObject)->Shutdown();
	std::vector<CAnimatedObject*>::iterator begin = mObjectList.begin() + m_iSelectedObject;

	mObjectList.erase(begin,begin + 1);
	m_iSelectedObject = 0;
}


void CAnimObjectMgr::AddObjectDlg(void)
{
	WCF::ModalWindow window;
	if(window.RunWindow(mhInstance,mhWnd,MAKEINTRESOURCE(IDD_DIALOG_ADD_ANIM_OBJECT)) == IDC_BUTTON_AAO_OK)
	{
		char* objectFilename,*animClip;
		wchar_t* textureDir;
		objectFilename = window.m_pcTextBuffer1;
		textureDir = window.m_pwcTextBuffer;
		animClip = window.m_pcTextBuffer2;
		PositionData data;
		std::string temp(objectFilename);
		std::string b("data/Models/Anim/");
		temp = b + temp;
		std::wstring a;
		(textureDir!=L"") ? (a = L"data/Textures/") : (a = textureDir);
		if(!AddObject(data,temp.c_str(),a.c_str(),animClip))
			DisplayError("Couln't create the object!\nChances are the object doesn't exist.");
	}
}


PositionData CAnimObjectMgr::GetPlayerCoordinates()
{
	WCF::AnimPropDialog::AnimPropData data = mObjectList.at(m_iSelectedObject)->GetAllData();
	return data.Data;
}