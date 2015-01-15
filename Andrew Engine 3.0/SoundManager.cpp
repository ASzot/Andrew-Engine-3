//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "SoundManager.h"


CSoundManager::CSoundManager(void)
{
	g_pSoundManager = this;
}


CSoundManager::~CSoundManager(void)
{
}


bool CSoundManager::Initialize()
{
	HRESULT result;
	// Initialize the direct sound interface pointer for the default sound device.
	result = DirectSoundCreate8(NULL, &m_pDirectSound, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
	result = m_pDirectSound->SetCooperativeLevel(DXUTGetHWND(), DSSCL_PRIORITY);
	if(FAILED(result))
	{
		return false;
	}

	ISoundResource* soundRes = new CDirectSoundResource(true);
	if ( !soundRes->VInitialize( std::string("data/Sound/GunShot.wav"), m_pDirectSound ) )
		return false;
	m_SoundResources["GunShot"] = soundRes;
	ISoundResource* rayShot = new CDirectSoundResource(false);
	if ( !rayShot->VInitialize( std::string("data/Sound/RayShot.wav"), m_pDirectSound ) )
		return false;
	m_SoundResources["RayShot"] = rayShot;

	return true;
}


//TODO:
// Implement all of these.
void CSoundManager::PauseAll()
{
}

void CSoundManager::StopAll()
{
}

void CSoundManager::ResumeAll()
{
}

void CSoundManager::Shutdown()
{
	for ( SoundResourceMap::iterator i = m_SoundResources.begin(); i != m_SoundResources.end(); ++i )
	{
		ISoundResource* s = i->second;
		s->VStop();
		if ( s )
		{
			s->Shutdown();
			delete s;
			s = 0;
		}
	}
	ReleaseCOM( m_pDirectSound );
}

ISoundResource* CSoundManager::GetSoundResouce(const char* name)
{
	SoundResourceMap::const_iterator found =  m_SoundResources.find(name);
	if ( found != m_SoundResources.end() ) 
	{
		return found->second;
	}

	return 0;
}

ISoundResource* g_fGetSoundResource(const char* filename)
{
	return g_pSoundManager->GetSoundResouce( filename );
}