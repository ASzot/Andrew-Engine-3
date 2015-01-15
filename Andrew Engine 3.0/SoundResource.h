//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once

#pragma comment(lib,"dsound.lib")

#include <string>
#include <dsound.h>
#include <stdio.h>
#include <MMSystem.h>
#include <Windows.h>

#ifndef ReleaseCOM
#define ReleaseCOM(x) { if(x) { x->Release(); x = 0; } } 
#endif

class CSoundInstance
{
public:
	CSoundInstance()
	{
	}
	virtual ~CSoundInstance()
	{
	}

	virtual bool VPlay(int volume, bool looping) = 0;

	virtual bool VStop() = 0;

	virtual bool VResume() = 0;

	virtual bool VTogglePause() = 0;

	virtual bool VIsPlaying() = 0;

	virtual bool VSetVolume(int volume) = 0;

};

class CDirectSoundInstance : public CSoundInstance
{
public:
	CDirectSoundInstance(IDirectSoundBuffer* secondaryBuffer)
		: CSoundInstance()
	{
		m_pSecondaryBuffer = secondaryBuffer;
	}
	virtual ~CDirectSoundInstance()
	{

	}

	virtual bool VPlay(int volume, bool looping);

	virtual bool VStop();

	virtual bool VResume();

	virtual bool VTogglePause();

	virtual bool VIsPlaying();

	virtual bool VSetVolume(int volume);

private:
	IDirectSoundBuffer* m_pSecondaryBuffer;
	int m_iVolume;
	bool m_bLooping;
	bool m_bPaused;

private:
	inline LONG ConvertRangeToDecibel( int in )
	{
		float coeff = (float)in / 100.0f;
		float logarithmicProportion = coeff > 0.1f ? 1 + log10(coeff) : 0;
		float range = (DSBVOLUME_MAX - DSBVOLUME_MIN);
		float fvolume = ( range * logarithmicProportion ) + DSBVOLUME_MIN;

		return LONG(fvolume);
	}
};


class ISoundResource
{
public:
	ISoundResource(void);
	virtual ~ISoundResource(void);

	virtual bool VInitialize(std::string filename, IDirectSound8* directSound) = 0;

	virtual void Shutdown() = 0;

	virtual void VStop() = 0;

	virtual CSoundInstance* GetSoundInstance() = 0;
};

class CDirectSoundResource : public ISoundResource
{
private:

	IDirectSoundBuffer*			m_pPrimaryBuffer;
	IDirectSound3DListener8*	m_pListener;
	IDirectSound3DBuffer8*		m_pSecondary3DBuffer;
	IDirectSoundBuffer8*		m_pSecondaryBuffer;
	// A pointer to the IDirectSound in SoundManager.
	IDirectSound8* m_ptDirectSound;

	bool	m_bPaused;
	int		m_iVolume;
	bool	m_bLooping;
	bool	m_bMultInstances;

private:
	struct WaveHeaderType
	{
		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
	};
public:
	CDirectSoundResource(bool allowMultipleInstances)
	{
		m_pPrimaryBuffer=0;
		m_pSecondaryBuffer=0;
		m_pListener=0;
		m_pSecondary3DBuffer=0;
		m_bLooping = false;
		m_iVolume = 50;
		m_bPaused = false;
		m_bMultInstances = allowMultipleInstances;
	}
	~CDirectSoundResource()
	{

	}

	virtual bool VInitialize(std::string filename, IDirectSound8* directSound);

	virtual void Shutdown();
	
	virtual CSoundInstance* GetSoundInstance();

	virtual void VStop();

private:
	bool InitDirectSound(IDirectSound8* directSound);
	bool LoadWaveFile(std::string filename, IDirectSound8* directSound);

};