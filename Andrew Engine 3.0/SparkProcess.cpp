//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "SparkProcess.h"
#include "EventManager.h"
#include "ParticleManager.h"

CSparkProcess::CSparkProcess(Vec& position)
	: CProcess( PROC_GRAPHICS_EFFECT )
{
	m_vPosition = position;
	m_pParticleSys = 0;
}

CSparkProcess::~CSparkProcess()
{

}

void CSparkProcess::VOnUpdate(const int deltaMilliseconds)
{
	CProcess::VOnUpdate( deltaMilliseconds );
}

void CSparkProcess::VOnInitialize()
{
	////These values are way off please change.
	//ID3D11Device* device = DXUTGetD3D11Device();
	//UINT maxParticles = 500;
	//float particleMin = 1.0f;
	//float particleMax = 1.25f;
	//float emitRate = 0.015f;
	//float particleLifeSpan = 10.0f;
	//float particleVel = 1.0f;
	////////////////////////////////
	//// Don't forget to actually set the filename to something.
	//LPCWSTR particleTexture = L"data/Particles/smokevol1.dds";
	//LPCWSTR colorLifespanTexture = L"data/Particles/colorgradient.dds";

	//m_pParticleSys = new ParticleSystem( 20.0f, 50.0f, 20.0f, true, false, true );

	//// Keep in mind that the particle manager will take care of memory deallocation.
	//m_pParticleSys->Initialize( device, maxParticles, particleMin, particleMax, emitRate, particleLifeSpan, particleVel, particleTexture, colorLifespanTexture,
	//	m_vPosition.X(), m_vPosition.Y(), m_vPosition.Z() );

	//m_idParticleSystem = g_GetEventManager()->GetParticleManager()->AddParticleSystem( m_pParticleSys );
}

void CSparkProcess::VKill()
{
	g_GetEventManager()->GetParticleManager()->RemoveParticleSystem( m_idParticleSystem );
}