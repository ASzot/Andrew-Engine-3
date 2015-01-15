//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "Process.h"


CProcess::CProcess(int ntype, unsigned int uOrder) : 
	m_iType( ntype ),
	m_bKill( false ),
	m_bActive( true ),
	m_uProcessFlags( 0 ),
	m_bPaused( false ),
	m_bInitialUpdate( true ),
	m_pNext( 0 )
{
}


CProcess::~CProcess(void)
{
}

void CProcess::VKill() 
{
	m_bKill = true;
}

void CProcess::SetNext(CProcess* pNext)
{
	m_pNext = pNext;
}

bool CProcess::IsAttached() const 
{
	return (m_uProcessFlags & PROCESS_FLAG_ATTACHED) ? true : false;
}

void CProcess::SetAttached(const bool wantAttached)
{
	if(wantAttached)
	{
		m_uProcessFlags |= PROCESS_FLAG_ATTACHED;
	}
	else
	{
		m_uProcessFlags &= PROCESS_FLAG_ATTACHED;
	}
}


void CProcess::VOnUpdate( const int miliseconds )
{
	if ( m_bInitialUpdate ) 
	{
		VOnInitialize();
		m_bInitialUpdate = false;
	}
}