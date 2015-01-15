//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "ProcessManager.h"

CProcessManager::CProcessManager()
{
	g_pProcessManager = this;
}

CProcessManager::~CProcessManager(void)
{

}

bool CProcessManager::HasProcesses()
{
	return !m_ProcessList.empty();
}

void CProcessManager::Attach(CProcess* pProcess)
{
	m_ProcessList.push_back( pProcess );
	pProcess->SetAttached( true );
}

void CProcessManager::Detach(CProcess* pProcess)
{
	m_ProcessList.remove( pProcess );
	m_DeleteList.push_front( pProcess );
	pProcess->SetAttached( false );
}

void CProcessManager::UpdateProcesses(int deltaMilliseconds)
{
	ProcessList::iterator i = m_ProcessList.begin();
	ProcessList::iterator end = m_ProcessList.end();

	CProcess* pNext;

	while ( i != end ) 
	{
		CProcess* p = *i;

		if ( p->IsDead() ) 
		{
			pNext = p->GetNext();
			if ( pNext )
			{
				p->SetNext( (CProcess*)(NULL) );
				Attach( pNext );
			}
			Detach( p );
			i = m_ProcessList.begin();
			if ( m_ProcessList.empty() )
				break;
		}

		else if ( p->IsActive() && !p->IsPaused() )
		{
			p->VOnUpdate( deltaMilliseconds );
		}
		++i;
	}
}

void CProcessManager::Shutdown()
{
	for ( ProcessList::iterator i = m_ProcessList.begin(); i != m_ProcessList.end(); )
	{
		// We have to manually kill all these processes becuase they are still in the updating list.
		(*i)->VKill();
		Detach( *(i++) );
	}

	for ( ProcessList::iterator i = m_DeleteList.begin(); i != m_DeleteList.end(); ++i )
	{
		CProcess* p = (*i);
		p->DeleteChildren();
		delete p;
		p = 0;
	}
}

CProcessManager* g_fGetProcessManager()
{
	return g_pProcessManager;
}