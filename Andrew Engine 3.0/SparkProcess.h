#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Process.h"
#include "ParticleSystem.h"

class CSparkProcess : public CProcess
{
public:
	CSparkProcess(Vec& position);
	~CSparkProcess();

	virtual void VOnUpdate(const int deltaMilliseconds);

	virtual void VOnInitialize();
	virtual void VKill();

private:
	ParticleSystem* m_pParticleSys;
	Vec m_vPosition;
	ObjectId m_idParticleSystem;
};