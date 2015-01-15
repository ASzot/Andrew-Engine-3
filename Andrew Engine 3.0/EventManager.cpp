//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "EventManager.h"
#include "System.h"
#include "AnimObjectMgr.h"
#include "ParticleManager.h"
#include "StaticObject.h"

CEventManager::CEventManager()
	: m_ptCam(0)
{
	//More than one of these better not be created
	gEventManager = this;
	m_bHasRegisteredMgr = false;
	m_ptParticleMgr = 0;
	mSelectedWaterSystem = -1;
	m_ptLightMgr = 0;
	m_SelectedMgr = eManager::eObjectMgr;
	m_BuilderPhysicsEnabled = false;
}

CEventManager::~CEventManager()
{

}

//Kind of deceiving because the message could be sent to multiple listeners
void CEventManager::SendMessageToListener(gEEventTypes listener, CHashedString message)
{
	if(listener == eSystemEventType)
	{
		if(message == "c2GameStatePaused")
			SetGameState(ePaused);
		else if(message == "c2GameStateResumed")
			SetGameState(eRunning);
		else if(message == "c2GameStateQuiting")
			SetGameState(eQuiting);
		else if(message == "c2GameStateSavingLevel")
			SetGameState(eSaving);
		else if(message == "c2GameStateOpeningLevel")
			SetGameState(eLoadingLevel);
	}

	for(int i = 0; i < mListenerList.size(); ++i)
	{
		if( mListenerList.at(i)->GetListenerType() == listener || message.BypassFilter )
			mListenerList.at(i)->RecieveMessage(message);
	}
}

void CEventManager::Process(CHashedString message)
{

}

void CEventManager::RegisterListener(IListener* listener)
{
	mListenerList.push_back(listener);
}


void CEventSnooper::SendMessageToListener(gEEventTypes listenerType, CHashedString eventType)
{
	gEventManager->SendMessageToListener(listenerType, eventType);
}

void CEventSnooper::ChangeGameState(GameStates gameState)
{
	if ( gameState != gEventManager->GetGameState() )
		gEventManager->SetGameState(gameState);
	else
		return;

	//Any of the states we would want a listener to know about
	switch(gameState)
	{
	case ePaused:
		gEventManager->SendMessageToListener(eSystemEventType,CHashedString("c2GameStatePaused"));
		break;
	case eRunning:
		gEventManager->SendMessageToListener(eSystemEventType,CHashedString("c2GameStateResumed"));
		break;
	}
}

void CEventSnooper::RecieveMessage(CHashedString eventType)
{
	//As a filter against any messages we might not want i.e. in the input stage or a nothing message
	if(eventType.GetEventType() == mListenerType || eventType.BypassFilter)
		mRecievingMessages.push_back(eventType);
}

CHashedString CEventSnooper::GetRecievedMessage()
{
	CHashedString message =  mRecievingMessages[0];
	mRecievingMessages.pop_back();
	return message;
}

void CEventManager::SetGameState(GameStates newGameState)
{
	//We have the chance to exit early, also gets rid of many errors that could have been possible
	if(newGameState == mCurrentGameState)
		return;

	if(newGameState == eLoading)
	{
		//For the loading message since the loading is going to be going on inside of a internal loop we create the loading screen now
		mSystemReference->OnLoad();
	}
	if(newGameState == eQuiting)
	{
		//Quiting is also a important thing that needs to be dealt with immediately
		mSystemReference->OnQuit();
	}
	if(mCurrentGameState == eLoading && newGameState == eRunning)
	{
		mSystemReference->OnFinishedLoading();
	}
	
	if(newGameState == eBuildingLevel)
	{
		mSystemReference->OnEnterBuilderMode();
	}
	if(newGameState == eRunning && mCurrentGameState == eBuildingLevel)
	{
		mSystemReference->OnExitBuilderMode();
	}
	

	mCurrentGameState = newGameState;
}

GameStates CEventManager::GetGameState()
{
	return mCurrentGameState;
}

void CEventManager::RegisterSystem(CSystem* system)
{
	mSystemReference = system;
}

void CEventManager::RegisterParticleMgr( CParticleManager* ptParticleManager )
{
	m_ptParticleMgr = ptParticleManager;
}

GameStates CEventSnooper::GetGameState()
{
	return gEventManager->GetGameState();
}

void CEventSnooper::SetSelectedObject(int s)
{
	gEventManager->mSelectedObject = s;
}

int CEventSnooper::GetSelectedObject()
{
	return gEventManager->mSelectedObject;
}

void CEventSnooper::CreateObject(const char* filename)
{
	gEventManager->mObjectBuffer = filename;
}

const char* CEventSnooper::GetCreatedObjectBuffer()
{
	return gEventManager->mObjectBuffer;
}

int CEventSnooper::GetSelectedLight()
{
	return gEventManager->mSelectedLight;
}

void CEventSnooper::SetSelectedLight(int s)
{
	gEventManager->mSelectedLight = s;
}

void CEventSnooper::CreateCharacter(SpawnInfo spawn,CPlayer* player)
{
	gEventManager->CreateCharacter(spawn,player);
}

void CEventManager::CreateCharacter(SpawnInfo info,CPlayer* player)
{
	if(m_bHasRegisteredMgr)
	{
		bool result = m_ptAnimObjMgr->AddObject(info.data,info.filename,info.texDir,info.animClip,player,info.bulletData);
		if(!result)
			FatalError("O god this isn't good.../nThere was an invalid object creation call!/nHave fun with all of the memory leaks.");
	}
	else
	{
		FatalError("Fool, you havn't registered the Animated Object Manager!");
	}
}

Camera* CEventSnooper::GetCameraReference()
{
	return gEventManager->ObtainReferenceToCamera();
}


bool* CEventSnooper::CreateLoadingDlg(int numberOfSteps)
{
	return gEventManager->mSystemReference->CreateLoadingScreen(numberOfSteps);
}

void CEventSnooper::StepLoadingDlg() 
{
	gEventManager->mSystemReference->StepLoadingDlg();
}

void CEventSnooper::SendObjectForRendering(CStaticObject* object)
{
	gEventManager->SendObjectForRendering( object );
}

int CEventSnooper::GetSelectedWaterSystem()
{
	return gEventManager->mSelectedWaterSystem;
}

void CEventSnooper::SetSelectedWaterSystem( int selectedWaterSystem )
{
	gEventManager->mSelectedWaterSystem = selectedWaterSystem;
}

void CEventManager::SendObjectForRendering(CStaticObject* object)
{
	// Checking that there actually is the manager.
	if ( m_bHasRegisteredMgr )
	{
		m_ptAnimObjMgr->AddObjectToRender( object );
	}
}

CSystem* CEventManager::GetSystem()
{
	return mSystemReference;
}

CParticleManager* CEventSnooper::GetParticleManager()
{
	return gEventManager->m_ptParticleMgr;
}

eManager CEventSnooper::GetSelectedMgr()
{
	return gEventManager->m_SelectedMgr;
}

void CEventSnooper::SetSelectedMgr(eManager newMgr)
{
	gEventManager->m_SelectedMgr = newMgr;
}

CParticleManager* CEventManager::GetParticleManager()
{
	return m_ptParticleMgr;
}

CEventManager* g_GetEventManager()
{
	return gEventManager;
}


void CEventManager::RegisterObjMgr( CObjectManager* ptObjMgr )
{
	m_ptObjMgr = ptObjMgr;
}	

CObjectManager* CEventManager::GetObjectManager()
{
	return m_ptObjMgr;
}

void CEventManager::RegisterLightMgr( CLightManager* ptLightManager )
{
	m_ptLightMgr = ptLightManager;
}

CLightManager* CEventManager::GetLightManager()
{
	if ( !m_ptLightMgr )
	{
		//DebugWriter::WriteToDebugFile( "Light manager not registered.", "CLightManager* CEventManager::GetLightManager()" );
		return 0;
	}
	return m_ptLightMgr;
}

void CEventManager::SendDirectMessage( CHashedString message )
{
	if ( message == "c#MgrObject" )
		m_SelectedMgr = eManager::eObjectMgr;
	else if ( message == "c#MgrLight" )
		m_SelectedMgr = eManager::eLightMgr;
	else if ( message == "c#MgrParicle" )
		m_SelectedMgr = eManager::eParticleMgr;
}

bool CEventManager::IsBuilderPhysicsEnabled()
{
	return m_BuilderPhysicsEnabled;
}

void CEventManager::SetIsBuilderPhysicsEnabled( bool enabled )
{
	m_BuilderPhysicsEnabled = enabled;
}
