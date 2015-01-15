//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once
#include "manager.h"
#include "ParticleSystem.h"
#include "ExplosionMgr.h"

class CParticleManager :
	public CManager
{
public:
	CParticleManager(HINSTANCE hInstance,HWND hwnd);
	virtual ~CParticleManager(void);


	bool Init(CEventManager* eventManager, CHashedString messageToWatch);
	bool InitializeResources(ID3D11Device* device, CEffectManager* effectManager);
	void Shutdown(void);
	void DestroyResources(void);
	void Update(double time, float elapsedTime,CameraInfo& cam,ID3D11Device* device,ID3D11DeviceContext* context);
	void Serilize(CDataArchiver* archiver);
	void OnDialogQuit();
	void OnResized(const DXGI_SURFACE_DESC* sd,ID3D11Device* device);

	bool LoadScene(ID3D11Device* device, std::vector<ParticleSystemInfo> psi, CEffectManager* effectManager);
	void DestroyGameAssets();

	void Render(ID3D11DeviceContext* context,CameraInfo& cam,CEffectManager* effectManager);

	ObjectId AddParticleSystem( ParticleSystem* particleSystem);
	void RemoveParticleSystem( ObjectId id );

	CParticleEffect* GetParticleEffect( std::string code, CEffectManager* effectManager );
	CTexture GetTextureOfEffect( std::string code, CEffectManager* effectManager );
	CTexture GetRandomTexture();

	void ListenToMessages(char* message);

	CExplosionMgr* GetExplosionMgr();

	
	void ProcessMessage(CHashedString message);

protected:
	BOOL CreateDlgWindow();

private:
	typedef std::vector<ParticleSystem*> ParticleSysList;
	ParticleSysList mParticleSystemList;
	bool CreateParticleSystem( XMFLOAT3 pos, XMFLOAT3 emitDir, float texSize, float vel, float randX, float randZ, UINT maxParticles );

	void CreateDefaultParticleSystem();
private:
	int m_iSelectedSystem;
	CTexture m_FireTex;
	CTexture m_SparkTex;
	CTexture m_RandomTex;
	CEffectManager* m_ptEffectManager;
	CExplosionMgr m_ExplosionMgr;
};

