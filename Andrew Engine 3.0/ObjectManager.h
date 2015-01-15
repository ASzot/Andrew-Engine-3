#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "StaticObject.h"
#include <vector>
#include "EventManager.h"
#include "SimpleObject.h"
#include "WindowCreatorFoundation.h"
#include "RenderSettings.h"
#include "GameObjMgr.h"
#include "WaterObject.h"

/////////////////////////
// Forward Declarations:
class CLightManager;

class CObjectManager : public CDataObject
{
public:
	CObjectManager(CEventManager* eventManager,HINSTANCE hInstance,CGameObjMgr* mgr);
	virtual ~CObjectManager();

	bool InitResources(ID3D11Device* device);
	void Shutdown();
	void DestroyResources();

	bool Initialize(CEventManager* eventManager,HWND hwnd);

	void Render(ID3D11DeviceContext* context,CameraInfo& cam,const CEffectManager* effectManager, ExtraRenderingData& erd);
	void RenderShadowMap( ID3D11DeviceContext* context, CameraInfo& cam, const CEffectManager* effectManager, CLightManager* pLightMgr );

	// Various rendering funcs.
	void RenderRefractableObjects( CameraInfo& cam, CEffectManager* effectManager );
	void RenderReflectableObjects( CameraInfo& cam, CEffectManager* effectManager, ExtraRenderingData& rtd, float waterHeight );

	bool LoadScene(ID3D11Device* device, vector<ObjectInfo> GeometryInfo);
	void DestroyGameAssets();

	void Update(GameStates gameStatem, bool flash);

	void Serilize(CDataArchiver* dataArchiver);

	void OnObjectClicked( ObjectId idOfObject );


private:
	vector<CStaticObject*> mObjectList;
	vector<CSimpleObject*> mSimpleObjectList;
	ID3D11RasterizerState* mNoCullRS;
	CEventSnooper mEventListener;
	int mSelectedObject;
	WCF::ObjectPropDialog mWindow;
	HINSTANCE mhInstance;
	HWND mhWnd;
	CGameObjMgr* m_ptObjMgr;

	// The window for the Object Physics Properties.
	WCF::ObjectPhysicsPropDialog mOPPWindow;
private:
	// Destroys and releases all current objects in the scene
	void DestroyObjects(void);
	void UpdateAllWindows();
	void CreateOPPWindow();
	void OnSelectedObjectChanged( int newSelectedObject );
public:
	void DeleteObject(int index);
	bool AddObject(const char* filename, ID3D11Device* device);
	bool AddObject(const char* filename, ID3D11Device* device,PositionData data,float tx,float ty,float tz);
	ObjectId InsertObject(CStaticObject* object);
	CStaticObject* GetObjectByIndex( int index );
};