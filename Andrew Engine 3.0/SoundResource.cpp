//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "SoundResource.h"


ISoundResource::ISoundResource(void)
{

}


ISoundResource::~ISoundResource(void)
{
}


bool CDirectSoundResource::VInitialize(std::string filename, IDirectSound8* directSound)
{
	m_ptDirectSound = directSound;
	if ( !InitDirectSound( directSound ) )
		return false;
	if ( !LoadWaveFile( filename, directSound ) )
		return false;

	return true;
}

void CDirectSoundResource::Shutdown()
{
	ReleaseCOM( m_pSecondaryBuffer );
	ReleaseCOM( m_pSecondary3DBuffer );
	ReleaseCOM( m_pPrimaryBuffer );

	ReleaseCOM( m_pListener );
}

bool CDirectSoundResource::InitDirectSound(IDirectSound8* pDirectSound)
{
	HRESULT result;
	DSBUFFERDESC bufferDesc;
	WAVEFORMATEX waveFormat;

	// Setup the primary buffer description.
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Get control of the primary sound buffer on the default sound device.
	result = pDirectSound->CreateSoundBuffer(&bufferDesc, &m_pPrimaryBuffer, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Setup the format of the primary sound bufffer.
	// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// Set the primary buffer to be the wave format specified.
	result = m_pPrimaryBuffer->SetFormat(&waveFormat);
	if(FAILED(result))
	{
		return false;
	}

	result=m_pPrimaryBuffer->QueryInterface(IID_IDirectSound3DListener8,(LPVOID*)&m_pListener);
	if(FAILED(result))
		return false;

	m_pListener->SetPosition(0.0f,0.0f,0.0f,DS3D_IMMEDIATE);

	return true;
}

bool CDirectSoundResource::LoadWaveFile(std::string f, IDirectSound8* directSound)
{
	IDirectSound3DBuffer8** ppTempBuffer3D = &m_pSecondary3DBuffer;
	IDirectSoundBuffer8** ppTempBuffer = &m_pSecondaryBuffer;
	const char* filename = f.c_str();
	errno_t error;
	FILE* filePtr;
	unsigned int count;
	WaveHeaderType waveFileHeader;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT hr;
	IDirectSoundBuffer* tempBuffer;
	unsigned char* waveData;
	unsigned char* bufferPtr;
	unsigned long bufferSize;

	error=fopen_s(&filePtr,filename,"rb");
	if(error!=0)
	{
		return false;
	}
	
	count=fread(&waveFileHeader,sizeof(waveFileHeader),1,filePtr);
	if(count!=1)
		return false;
	if((waveFileHeader.chunkId[0] != 'R') || (waveFileHeader.chunkId[1] != 'I') || 
	   (waveFileHeader.chunkId[2] != 'F') || (waveFileHeader.chunkId[3] != 'F'))
	{
		return false;
	}
 
	if((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
	   (waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E'))
	{
		return false;
	}

	if((waveFileHeader.subChunkId[0] != 'f') || (waveFileHeader.subChunkId[1] != 'm') ||
	   (waveFileHeader.subChunkId[2] != 't') || (waveFileHeader.subChunkId[3] != ' '))
	{
		return false;
	}

	if(waveFileHeader.audioFormat!=WAVE_FORMAT_PCM)
		return false;

	//if(waveFileHeader.numChannels!=1)
	//	return false;
	//if(waveFileHeader.sampleRate!=44100)
	//	return false;
	////Be sure that the wave file was recorded in a 16 bit format!
	//if(waveFileHeader.bitsPerSample!=16)
	//	return false;

	//if((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
	//   (waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a'))
	//{
	//	return false;
	//}

	waveFormat.wFormatTag=WAVE_FORMAT_PCM;
	waveFormat.wBitsPerSample=waveFileHeader.bitsPerSample;
	waveFormat.nSamplesPerSec=waveFileHeader.sampleRate;
	waveFormat.cbSize=0;
	waveFormat.nChannels=1;
	waveFormat.nBlockAlign=(waveFormat.wBitsPerSample/8)*waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec=waveFormat.nSamplesPerSec*waveFormat.nBlockAlign;

	bufferDesc.dwSize=sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags=DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
	bufferDesc.dwBufferBytes=waveFileHeader.dataSize;
	bufferDesc.dwReserved=0;
	bufferDesc.lpwfxFormat=&waveFormat;
	bufferDesc.guid3DAlgorithm=GUID_NULL;
	
	hr=directSound->CreateSoundBuffer(&bufferDesc,&tempBuffer,NULL);
	if(FAILED(hr))
		return false;

	hr=tempBuffer->QueryInterface(IID_IDirectSoundBuffer8,(void**)&*ppTempBuffer);
	if(FAILED(hr))
		return false;
	tempBuffer->Release();
	tempBuffer=0;

	fseek(filePtr,sizeof(WaveHeaderType),SEEK_SET);
	waveData=new unsigned char[waveFileHeader.dataSize];
	if(!waveData)
		return false;

	count=fread(waveData,1,waveFileHeader.dataSize,filePtr);
	if(count!=waveFileHeader.dataSize)
		return false;

	error=fclose(filePtr);
	if(error!=0)
		return false;

	//Here we are locking the secondary buffer so the sound data can be copied and written to it
	hr=(*ppTempBuffer)->Lock(0,waveFileHeader.dataSize,(void**)&bufferPtr,(DWORD*)&bufferSize,NULL,0,0);
	if(FAILED(hr))
		return false;

	memcpy(bufferPtr,waveData,waveFileHeader.dataSize);

	hr=(*ppTempBuffer)->Unlock((void*)bufferPtr,bufferSize,NULL,0);
	if(FAILED(hr))
		return false;

	delete [] waveData;
	waveData=0;

	//We aquire the 3D sound controls from the regular secondary buffer although all the other functions such as volume need to be set in the original sound buffer interface.
	hr=(*ppTempBuffer)->QueryInterface(IID_IDirectSound3DBuffer8,(void**)&*ppTempBuffer3D);
	if(FAILED(hr))
		return false;

	return true;
}

CSoundInstance* CDirectSoundResource::GetSoundInstance()
{
	if ( m_bMultInstances ) 
	{
		LPDIRECTSOUNDBUFFER tempBuff;
		m_ptDirectSound->DuplicateSoundBuffer( m_pSecondaryBuffer, &tempBuff );
		return new CDirectSoundInstance( tempBuff );
	}
	else
		return new CDirectSoundInstance( m_pSecondaryBuffer );
}
void CDirectSoundResource::VStop()
{
	// Should do nothing... the instances should be doing this.
}


bool CDirectSoundInstance::VPlay(int volume, bool looping)
{
	//VStop();
	m_iVolume = volume;
	m_bLooping = looping;


	m_pSecondaryBuffer->SetVolume( ConvertRangeToDecibel( volume ) );

	DWORD flags = looping ? DSBPLAY_LOOPING : 0L;

	m_pSecondaryBuffer->SetCurrentPosition( 0 );
	return (S_OK==m_pSecondaryBuffer->Play( 0, 0, flags ) ); 
}

bool CDirectSoundInstance::VStop()
{
	m_bPaused = true;
	if ( FAILED( m_pSecondaryBuffer->Stop() ) )
		return false;
	return true;
}

bool CDirectSoundInstance::VResume()
{
	m_bPaused = false;
	return VPlay( m_iVolume, m_bLooping );
}

bool CDirectSoundInstance::VTogglePause()
{
	bool result = true;
	if ( m_bPaused )
	{
		result = VResume();
	}
	else
	{
		result = VStop();
	}

	return result;
}

bool CDirectSoundInstance::VIsPlaying()
{
	DWORD dwStatus = 0;
	m_pSecondaryBuffer->GetStatus( &dwStatus );
	bool bIsPlaying = ( ( dwStatus & DSBSTATUS_PLAYING ) != 0 );

	return bIsPlaying;
}

bool CDirectSoundInstance::VSetVolume(int volume)
{
	assert( volume >= 0 && volume <= 100 && "Volume must be between 0 and 100!" );
	if ( FAILED( m_pSecondaryBuffer->SetVolume( ConvertRangeToDecibel( volume ) ) ) )
		return false;
	m_iVolume = volume;
	return true;
}