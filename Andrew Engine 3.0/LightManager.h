#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================


#include "StaticObject.h"
#include "tEvent.h"
#include "resource.h"
#include "WindowCreatorFoundation.h"
#include "EventManager.h"
#include "ShadowPointLight.h"

// Used in the LookAt field to alert others that the data is actually a shadow point light rather then a regular point light.
#define SHADOW_PL 696969
#define NO_SHADOW_PL 9696969.123

#ifndef IS_NO_SHADOW_PL
#define IS_NO_SHADOW_PL(pl) { ( pl.LookAt.x == NO_SHADOW_PL && pl.LookAt.y == NO_SHADOW_PL && pl.LookAt.z == NO_SHADOW_PL ) }
#endif

class CLightManager : public CDataObject
{
public:
	CLightManager(EGraphicsSettings settings);
	virtual ~CLightManager(void);

	bool Initialize(HINSTANCE hInstance, HWND hwnd, CEventManager* eventManager);

	virtual void Shutdown();

	virtual void Serilize(CDataArchiver* dataArchiver);
	
	void Update(GameStates gameState,float dt);

	void DestroyResources();

	// This is based off of the light's parameters and is for the 'regular' rendering.
	void GeneratePointLightView( int index );


	CSceneLighting GetLights()
	{
		return mLightingInfo;
	}

	void Render(ID3D11DeviceContext* context, CEffectManager*& effectManager,CameraInfo& cam,CSkinEffect* skinEffect);

	XMMATRIX GetLightView();
	XMMATRIX GetLightProj();
	XMMATRIX GetShadowTransform();
	
	// This is for a forced view matrix 
	XMMATRIX ConstructPointLightView( int index, Vec& lookAt );
	XMMATRIX ConstructShadowTransform( CXMMATRIX view, CXMMATRIX proj );

	void RenderLightSymbols(ID3D11DeviceContext* context, CEffectManager* effectManager,CameraInfo& cam);

	PointLight* CreatePointLight( PointLight pointLight );

	int GetNumberOfPointLights();

private:
	CSceneLighting mLightingInfo;
	std::vector<CShadowPointLight> m_ShadowPointLights;
	bool mIsSceneLoaded;
	std::vector<CStaticObject*> mLightSymbols;
	WCF::LightPropDialog mWindow;
	HINSTANCE mhInstance;
	HWND mhWnd;
	CEventSnooper mEventListener; 
	int mSelectedLight;
	XMFLOAT4X4 m_4x4LightProj;
	XMFLOAT4X4 m_4x4LightView;
	XMFLOAT4X4 m_4x4ShadowTransform;
	float m_fLightRotationAngle;
	Vec m_vOriginalLightDir;
public:
	// Loads a new lighting scene and all lights based off the lighting settings
	bool LoadScene(CSceneLighting setttings);
	void DestroyGameAssets();

private:
	
	void DestroySymbols(void);
	void AddPointLight();
	void RemovePointLight(int index);
	void UpdateShadowTransform();
};

