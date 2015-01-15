//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "GameProcesses.h"
#include "EventManager.h"
#include "ParticleManager.h"
#include "ExplosionMgr.h"

CExplosionFXProcess::CExplosionFXProcess(XMFLOAT3& pos)
	: CProcess( PROC_GRAPHICS_EFFECT )
{
	m_vPosition = pos;
}

CExplosionFXProcess::~CExplosionFXProcess()
{

}

void CExplosionFXProcess::VOnUpdate( const int deltaMilliseconds )
{
	CProcess::VOnUpdate( deltaMilliseconds );

	// Immediately kill this process until we come up with more for this to do.
	VKill();
}

void CExplosionFXProcess::VKill()
{
	CProcess::VKill();
}

void CExplosionFXProcess::VOnInitialize()
{
	CExplosionMgr* explosionMgr = g_GetEventManager()->GetParticleManager()->GetExplosionMgr();
	explosionMgr->AddExplosion( MineExplosionType, Vec( m_vPosition ) );
}
