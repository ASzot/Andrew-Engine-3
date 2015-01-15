#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "tEvent.h"
#include "Spawning.h"

class CStaticObject;
class CSystem;
class CAnimObjectMgr;
class CParticleManager;
class CObjectManager;
class CLightManager;

// You should be talking to event manager through the event snooper ( or some other child of ilistener )
class CEventSnooper : public IListener
{
public:
	CEventSnooper(gEEventTypes listenerType)
		: IListener(listenerType)
	{

	}
	virtual ~CEventSnooper()
	{ }

	virtual void SendMessageToListener(gEEventTypes listenerType, CHashedString eventType);

	void ChangeGameState(GameStates newGameState);

	GameStates GetGameState();

	void SetSelectedObject(int selectedObject);
	void SetSelectedLight(int selectedLight);
	void SetSelectedWaterSystem( int selectedWaterSystem );

	void CreateCharacter(SpawnInfo spawn,CPlayer* player);
	CParticleManager* GetParticleManager();

	void CreateObject(const char* filename);
	const char* GetCreatedObjectBuffer();

	int GetSelectedObject();
	int GetSelectedLight();
	int GetSelectedWaterSystem();
	
	virtual void RecieveMessage(CHashedString eventType);
	virtual CHashedString GetRecievedMessage();

	// This returns a 'handle' on when the dialog should be shut.
	// Setting it to false will shut down the dialog.
	bool* CreateLoadingDlg(int numberOfSteps);
	void StepLoadingDlg();

	eManager GetSelectedMgr();
	void SetSelectedMgr(eManager newMgr);

	Camera* GetCameraReference();

	void SendObjectForRendering(CStaticObject* object);
};


class CEventManager
{
private:
	vector<IListener*> mListenerList;
	GameStates mCurrentGameState;
	// A pointer to the system of the game engine the pause event and other super important events.
	CSystem* mSystemReference;
	int mSelectedObject;
	int mSelectedLight;
	int mSelectedWaterSystem;
	const char* mObjectBuffer;
	// For creating characters from the character manager.
	CAnimObjectMgr* m_ptAnimObjMgr;
	CObjectManager* m_ptObjMgr;
	CLightManager* m_ptLightMgr;
	// So we can easily register particle effects on the fly.
	CParticleManager* m_ptParticleMgr;
	bool m_bHasRegisteredMgr;
	Camera* m_ptCam;
	eManager m_SelectedMgr;
	bool m_BuilderPhysicsEnabled;

public:
	CEventManager();
	~CEventManager();

	void SendMessageToListener(gEEventTypes listener, CHashedString message);

	void Process(CHashedString message);

	void RegisterListener(IListener* listener);

	void ChangeSelectedMgr( eManager newMgr );

	void SendObjectForRendering( CStaticObject* object );

	bool IsBuilderPhysicsEnabled();
	void SetIsBuilderPhysicsEnabled(bool enabled);

	void RegisterCamera(Camera* cam)
	{
		m_ptCam = cam;
	}

	void SendDirectMessage( CHashedString message );

	Camera* ObtainReferenceToCamera()
	{
		// The camera hasn't been initialized yet.
		if ( !m_ptCam )
			FatalError("The pointer to the camera is invalid!\n( Camera* CEventManager::ObtainReferenceToCamera() )");
		return m_ptCam;
	}

	CSystem* GetSystem();

	void SetGameState(GameStates updatedGameState);

	GameStates GetGameState();

	void RegisterSystem(CSystem* system);
	void RegisterAnimObjMgr(CAnimObjectMgr* mgr)
	{
		m_ptAnimObjMgr = mgr;
		m_bHasRegisteredMgr = true;
	}

	void RegisterParticleMgr( CParticleManager* ptParticleMgr );
	CParticleManager* GetParticleManager();

	void  RegisterObjMgr( CObjectManager* ptObjMgr );
	CObjectManager* GetObjectManager();

	void RegisterLightMgr( CLightManager* ptLightMgr );
	CLightManager* GetLightManager();

	void CreateCharacter(SpawnInfo info,CPlayer* player);

	friend eManager CEventSnooper::GetSelectedMgr();
	friend void CEventSnooper::SetSelectedMgr(eManager newMgr);
	friend void CEventSnooper::SetSelectedObject(int selectedObject);
	friend void CEventSnooper::SetSelectedLight(int selectedLight);
	friend void CEventSnooper::SetSelectedWaterSystem( int selectedWaterSystem );
	friend int CEventSnooper::GetSelectedObject();
	friend int CEventSnooper::GetSelectedLight();
	friend int CEventSnooper::GetSelectedWaterSystem();
	friend void CEventSnooper::CreateObject(const char* filename);
	friend const char* CEventSnooper::GetCreatedObjectBuffer();
	friend void CEventSnooper::CreateCharacter(SpawnInfo spawn,CPlayer* player);
	friend bool* CEventSnooper::CreateLoadingDlg(int numberOfSteps);
	friend void CEventSnooper::StepLoadingDlg();
	friend CParticleManager* CEventSnooper::GetParticleManager();
};

namespace
{
	CEventManager* gEventManager;
}

CEventManager* g_GetEventManager();




