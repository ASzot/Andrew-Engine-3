#include "DXUT.h"
#include "PhysicsManager.h"


CPhysicsManager::CPhysicsManager(HINSTANCE hInstance, HWND hwnd)
	: CManager( hInstance, hwnd, gEEventTypes::eInputEventType, new WCF::KinematicsWindow() )
{
}


CPhysicsManager::~CPhysicsManager(void)
{
}


void CPhysicsManager::Update(float dt,double tt)
{
	CManager::Update( dt );

	// Play around with this dt value.
	bool updatePhysics = ( mEventListener.GetGameState() == eRunning );
	if ( mEventListener.GetGameState() == eBuildingLevel && g_GetEventManager()->IsBuilderPhysicsEnabled() )
		updatePhysics = true;

	if ( updatePhysics )
	{
		m_pPhysicsSystem->VOnUpdate( dt );
		m_pPhysicsSystem->VSyncVisibleScene( m_ptGameObjMgr );
	}
}


bool CPhysicsManager::Init(CGameObjMgr* mgr, CEventManager* eventMgr)
{
	CManager::Init( eventMgr, CHashedString( "c1CreatePhysicsWindow" ) );
	m_pPhysicsSystem = CreateGamePhysics( g_bUseNullPhysics, -5.0f );
	assert(m_pPhysicsSystem);


	mgr->RegisterPhysicsSystem( m_pPhysicsSystem );

	m_ptGameObjMgr = mgr;
	return true;
}


void CPhysicsManager::Shutdown(void)
{
	CManager::Shutdown();
	DestroyObject( m_pPhysicsSystem );
}


void CPhysicsManager::AddPhysicsObject( IObject* obj, rVec dim, float specGrav, enum PhysicsMaterial mat )
{
	m_pPhysicsSystem->VAddBox( dim, obj, specGrav, mat );
}

void CPhysicsManager::Render( )
{
	m_pPhysicsSystem->VRenderDiagnostics( );
}

bool CPhysicsManager::InitResources(ID3D11Device* device,Camera* cam,CGameObjMgr* gameObjMgr)
{
	return m_pPhysicsSystem->VInitialize( gameObjMgr );
}

void CPhysicsManager::CastRay( int x, int y, Camera& cam, UINT flags, bool straightForward )
{
	int screenHeight = (int)DXUTGetWindowHeight();
	int screenWidth = (int)DXUTGetWindowWidth();

	XMMATRIX view = cam.View();
	XMMATRIX proj = cam.Proj();

	float PRVecX, PRVecY, PRVecZ;

	XMVECTOR pickRayInViewSpaceDir = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
	XMVECTOR pickRayInViewSpacePos = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );

	PRVecX = ( ( ( 2.0f * x ) / screenWidth ) - 1 ) / proj( 0, 0 );
	PRVecY = -( ( ( 2.0f * y ) / screenHeight ) - 1 ) / proj( 1, 1 );
	PRVecZ = 1.0f;

	if ( !straightForward )
		pickRayInViewSpaceDir = XMVectorSet( PRVecX * 1000, PRVecY * 1000, PRVecZ * 1000, 0.0f );
	else
		pickRayInViewSpaceDir = XMVectorSet(0.0f, 0.0f, 100.0f, 0.0f);

	XMMATRIX pickRayToWorldSpaceMatrix;
	XMVECTOR matInvDeter;

	// The inverse of the view is the world matrix.
	pickRayToWorldSpaceMatrix = XMMatrixInverse( &matInvDeter, view );

	XMVECTOR pickRayInWorldSpacePos = XMVector3TransformCoord( pickRayInViewSpacePos, pickRayToWorldSpaceMatrix );
	XMVECTOR pickRayInWorldSpaceDir = XMVector3TransformCoord( pickRayInViewSpaceDir, pickRayToWorldSpaceMatrix );

	Vec at( pickRayInWorldSpacePos );
	Vec to( pickRayInWorldSpaceDir );

	m_pPhysicsSystem->VCastRay( Vec( cam.GetPosition() ), to, flags );
}

void CPhysicsManager::Serilize( CDataArchiver* archiver )
{
	// Serilize all of the constraints.
	//std::map<ObjectId, BulletConstraint*> allConstraints = m_pPhysicsSystem->VGetAllConstraints();
	//for ( std::map<ObjectId, BulletConstraint*>::iterator i = allConstraints.begin(); i != allConstraints.end(); ++i )
	//{
	//	archiver->WriteToStream('c');
	//	archiver->Space();
	//	archiver->WriteToStream(i->first);
	//	archiver->Space();
	//	i->second->Serilize( archiver );
	//	archiver->Space();
	//	archiver->WriteToStream('\n');
	//}
}

// For window messages.
void CPhysicsManager::ListenToMessages(char* message)
{
}

// For event messages.
void CPhysicsManager::ProcessMessage(CHashedString message)
{

}

BOOL CPhysicsManager::CreateDlgWindow()
{
	WCF::KinematicsWindow* window = ( WCF::KinematicsWindow* )mWindow;
	KinematicsInfo ki;
	ki.AppliedForce = 0.0f;
	ki.AppliedTorque = 0.0f;
	ki.Constraint = 0;
	ObjectId selectedObject = mEventListener.GetSelectedObject();
	ki.SelectedObject = selectedObject;
	ki.LinearVelocity = m_pPhysicsSystem->VGetVelocity(selectedObject).Round().ToFloat3();
	ki.AngularVelocity = m_pPhysicsSystem->VGetAngularVelocity(selectedObject).Round().ToFloat3();

	if ( m_pPhysicsSystem->VGetNumberOfConstraints( selectedObject ) != 0 )
	{
		ki.HasConstraint = true;
		ki.Constraint = m_pPhysicsSystem->VGetConstraint(selectedObject);
	}
	else
		ki.HasConstraint = false;

	return window->RunWindow( mhInstance, mhWnd, ki );
}


void CPhysicsManager::OnDialogQuit()
{
	WCF::KinematicsWindow* window = ( WCF::KinematicsWindow* )mWindow;
	KinematicsInfo ki = window->GetWindowData();
	ObjectId selectedObject = ki.SelectedObject;

	m_pPhysicsSystem->VSetAngularVelocity( ki.SelectedObject, Vec( ki.AngularVelocity ) );
	m_pPhysicsSystem->VSetVelocity( ki.SelectedObject, Vec( ki.LinearVelocity ) );

	if ( ki.HasConstraint )
	{
		//Check if it is the same constraint as before or if it is a new one.
		BulletConstraint* bc = window->GetConstraint();
		if ( bc )
		{
			// This is a new edited constraint.

			// Remove the old constraint, if it was old.
			if ( m_pPhysicsSystem->VGetConstraint( ki.SelectedObject ) != bc )
				m_pPhysicsSystem->VRemoveConstraints( ki.SelectedObject );
			// New constraint.
			m_pPhysicsSystem->VAddConstraint(bc);

			window->ResetConstraint();
		}
	}
	else
	{
		// Check if there used to be a constraint.
		if ( m_pPhysicsSystem->VGetNumberOfConstraints( selectedObject ) != 0 )
		{
			// There used to be a constraint delete it.
			m_pPhysicsSystem->VRemoveConstraints(selectedObject);
		}
	}
}

void CPhysicsManager::Restart()
{
	DestroyObject( m_pPhysicsSystem );

	CGameObjMgr* objMgr = g_GetGameObjMgr();

	m_pPhysicsSystem = CreateGamePhysics( g_bUseNullPhysics, -5.0f );
	assert( m_pPhysicsSystem );

	m_pPhysicsSystem->VInitialize( objMgr );

	m_pPhysicsSystem->ResyncPhysics(objMgr);

	objMgr->RegisterPhysicsSystem( m_pPhysicsSystem );
}
