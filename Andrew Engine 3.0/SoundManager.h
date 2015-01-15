//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once

#include "SoundResource.h"
#include <map>


class CSoundManager
{
private:
	typedef std::map<const char*, ISoundResource*> SoundResourceMap;
	SoundResourceMap m_SoundResources;

public:
	CSoundManager(void);
	~CSoundManager(void);

	bool Initialize();

	void PauseAll();
	void StopAll();
	void ResumeAll();

	void Shutdown();

	ISoundResource* GetSoundResouce(const char* name);
};

namespace 
{
	CSoundManager* g_pSoundManager;
	IDirectSound8* m_pDirectSound;
};


ISoundResource* g_fGetSoundResource(const char* filename);
