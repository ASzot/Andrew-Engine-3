#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Manager.h"

// Forward declarations.
class RenderEffects;
class CWaterObject;

class CWaterManager : public CManager
{
private:
	CTexture* m_pWaterNormalMap;

	typedef std::vector<CWaterObject*> WaterObjectList;
	WaterObjectList m_WaterSystemList;
	const char* m_cPlaneModelName;
	const char* m_cCircleModelName;

	int m_iSelectedSystem;

public:
	CWaterManager(HINSTANCE hInstance, HWND hwnd);
	~CWaterManager(void);
protected:
	BOOL CreateDlgWindow(void);
	virtual void OnDialogQuit(void);

	void IncSelectedSystem();
	void DecSelectedSystem();
public:
	virtual void Serilize(CDataArchiver* archiver);
	bool Init(CEventManager* eventManager);
	bool InitializeResources(ID3D11Device* device);
	void Shutdown(void);
	void DestroyResources(void);
	void Update(float dt);

	void DestroyGameAssets();

	bool LoadScene( std::vector< WaterSystemOptions > wso );

	bool AddWaterSystem();
	bool AddWaterSystem( float x, float y, float z, float rx, float ry, float rz, float s, float texTrans, float waterScale );

	virtual void ListenToMessages(char* message);
	virtual void ProcessMessage(CHashedString message);

	void DeleteSelectedWaterSystem();
	void DeleteWaterSystem( int index );

	///<summary>The refraction texture should be at position 0 in array reflection should be at index 1, all other indexes are out of bounds.</summary>
	void RenderWaterObjects( CEffectManager* effectManager, CameraInfo& info, ID3D11ShaderResourceView** ppRefractionReflectionTexture, RenderEffects* pRenderEffects );
};

