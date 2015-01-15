#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Process.h"
#include "SoundResource.h"
#include <xnamath.h>

class CWaitProcess : public CProcess
{
protected:
	unsigned int m_uStart;
	unsigned int m_uEnd;
	unsigned int m_uNumMil;

public:
	CWaitProcess(unsigned int numMill)
		: CProcess( PROC_WAIT, 0 ),
		m_uEnd( numMill ),
		m_uStart( 0 ),
		m_uNumMil( numMill )
	{

	}

	virtual void VOnUpdate(const int deltaMilliseconds)
	{
		CProcess::VOnUpdate( deltaMilliseconds );
		if ( m_bActive )
		{
			m_uStart += deltaMilliseconds;
			if ( m_uStart >= m_uEnd )
				VKill();
		}
	}

	CWaitProcess* Clone()
	{
		return new CWaitProcess( m_uNumMil );
	}
};


class CSoundFXProcess : public CProcess
{
private:
	// The sound manager will take care of memory deallocation.
	CSoundInstance* m_pSoundResource;
	int m_iVolume;

public:
	CSoundFXProcess(int volume,CSoundInstance* res)
		: CProcess( PROC_SOUNDFX, 0 ),
		m_iVolume( volume )
	{
		if ( res )
			m_pSoundResource = res;
	}
	~CSoundFXProcess()
	{

	}

	virtual void VOnUpdate(const int deltaMilliseconds)
	{
		CProcess::VOnUpdate( deltaMilliseconds );

		if ( !m_bInitialUpdate && !m_pSoundResource->VIsPlaying() )
			VKill();
	}

	virtual void VOnInitialize()
	{
		m_pSoundResource->VPlay( m_iVolume, false );
	}

	virtual void VKill()
	{
		if ( m_pSoundResource->VIsPlaying() )
			m_pSoundResource->VStop();
		delete m_pSoundResource;
		m_pSoundResource = 0;
		CProcess::VKill();
	}
};

class CExplosionFXProcess : public CProcess
{
public:
	CExplosionFXProcess(XMFLOAT3& pos);
	~CExplosionFXProcess();

	virtual void VOnUpdate( const int deltaMilliseconds );

	virtual void VOnInitialize();

	virtual void VKill();

private:
	XMFLOAT3 m_vPosition;
};