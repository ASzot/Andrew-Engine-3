//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "GameLogic.h"

bool CGameLogic::Initialize(CEventManager* eventManager,HINSTANCE instance,HWND mainWindow)
{
	m_bFirstLoad = false;
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;
	m_dTimeLastFrame = 0;

	DebugWriter::DebugWriter();
	if ( DebugWriter::InitStream() )
		DebugWriter::WriteToDebugFile( "Up and running!", "bool CGameLogic::Initialize(CEventManager* eventManager,HINSTANCE instance,HWND mainWindow)" );

	m_pSoundManager = new CSoundManager();
	assert( m_pSoundManager );

	m_pProcessManager = new CProcessManager();
	assert( m_pProcessManager );


	mScene = new CScene();
	assert(mScene);
	
	mCam.SetPosition(0.0f,1.0f,-10.0f);
	mTHCam.SetPosition(XMFLOAT4(0.0f,10.0f,8.0f,0.0f));
	mTHCam.SetTarget(XMFLOAT3(0.0f,3.0f,0.0f));

	// Register the camera with the system so it can be accessed later elsewhere.
	eventManager->RegisterCamera( &GetCamera() );

	if(!mScene->Init(eventManager,instance,mainWindow))
		return false;

	return true;
}

void CGameLogic::Frame(double fTime, float fElapsedTime,GameStates gameState,ID3D11Device* device,ID3D11DeviceContext* context)
{
	float deltaTime = (fTime - m_dTimeLastFrame) * 1000;
	m_pProcessManager->UpdateProcesses( (int)deltaTime );

	CameraInfo cam;
	mCam.UpdateViewMatrix();
	cam.LookAt = mCam.GetLook();
	cam.Position = mCam.GetPosition();
	cam.Proj = mCam.Proj();
	cam.Yaw = mCam.GetYaw();
	cam.Pitch = mCam.GetPitch();
	cam.View = mCam.View();
	XMFLOAT3 right = mCam.GetRight();
	cam.Right = XMFLOAT4(right.x,right.y,right.z,0.0f);

	mScene->Update(gameState,fTime,fElapsedTime,cam,device,context);
	mCam.SetPosition(cam.Position);
	m_dTimeLastFrame = fTime;
}

bool CGameLogic::InitResources(ID3D11Device* device,const DXGI_SURFACE_DESC* sd,CEventManager* eventManager)
{
	if( !m_pSoundManager->Initialize() )
	{
		FatalError("Couldn't load the sound resources!");
		return false;
	}
	if(!mScene->InitResources(device,sd,&GetCamera(),eventManager))
		return false;

	return true;
}

void CGameLogic::Shutdown()
{
	DestroyObject( mScene );
	delete m_pSoundManager;
	m_pSoundManager = 0;
	DebugWriter::CloseStream();
}

void CGameLogic::DestroyResources()
{
	ReleaseResources(mScene);
	// Since we need the sound manager when destroying process manager.
	DestroyObject( m_pProcessManager );
	// Since Shutdown destroys all of the sound resources.
	m_pSoundManager->Shutdown();
}

void CGameLogic::Render(GameStates gameState)
{
	CameraInfo cam;
	mCam.UpdateRenderReflection( 1.0f );

	cam.LookAt = mCam.GetLook();
	cam.Position = mCam.GetPosition();
	cam.Proj = mCam.Proj();
	cam.View = mCam.View();
	cam.Yaw = mCam.GetYaw();
	cam.Pitch = mCam.GetPitch();
	cam.Refl = mCam.ReflectionView();
	
	mScene->Render(DXUTGetD3D11DeviceContext(),cam);
}

void CGameLogic::OnResize(const DXGI_SURFACE_DESC* sd,ID3D11Device* device)
{
	float fAspectRatio = sd->Width / ( FLOAT )sd->Height;
	mCam.SetLens(0.25f*3.1415926535f,fAspectRatio,0.1f,1000.0f,sd->Width,sd->Height);
	mScene->OnResize(sd,device);
	mTHCam.SetLens(((float)sd->Width)/((float)sd->Height),0.1f,1000.0f);
}

void CGameLogic::HandleMessages(HWND hwnd,UINT uMsg,WPARAM wParam, LPARAM lParam)
{
	// The message proc for our main window.
}

void CGameLogic::OnMouseDown(WPARAM mouseState,int x,int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;


	SetCapture(DXUTGetHWND());
}

void CGameLogic::OnMouseUp(WPARAM mouseState,int x, int y)
{
	ReleaseCapture();
}

void CGameLogic::OnMouseMove(WPARAM mouseState,int x, int y)
{
	if((mouseState & MK_RBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);

		if((x != mLastMousePos.x) || (y != mLastMousePos.y))
		{
			float cX = x - mLastMousePos.x;
			float cY = y - mLastMousePos.y;
			mTHCam.SetYaw( ( mTHCam.GetYaw() + ( cX * 0.000002f ) ) );
			mTHCam.SetPitch( (mTHCam.GetPitch() + ( cY * 0.002f ) ) );
		}
	}
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void CGameLogic::Serilize(CDataArchiver* arch)
{
	mScene->Serilize(arch);
}

bool CGameLogic::LoadScene(const char* filename, ID3D11Device* device)
{
	if ( !m_bFirstLoad )
	{
		Unload();
	}

	m_bFirstLoad = false;
	return mScene->LoadScene(filename,device);
}


void CGameLogic::OnLeftMouseDown(WPARAM mouseState,int x,int y)
{
	mScene->OnLeftMouseBtnDown( mouseState, x, y, mCam );
}

void CGameLogic::Unload()
{
	m_pProcessManager->Shutdown();
}
