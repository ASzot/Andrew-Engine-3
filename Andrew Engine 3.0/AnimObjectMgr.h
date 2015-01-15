#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "manager.h"
#include "AnimatedObject.h"
#include "GameObjMgr.h"




class CAnimObjectMgr :
	public CManager
{
public:
	CAnimObjectMgr();
	~CAnimObjectMgr(void);

private:
	BOOL CreateDlgWindow(void);
	void OnDialogQuit(void);
	void DeleteSelectedObject(void);
	void AddObjectDlg(void);

public:
	virtual bool Init(CEventManager* eventManager, CHashedString messageToWatch,CGameObjMgr* ctrlMgr);
	virtual bool InitializeResources(ID3D11Device* device);
	virtual void Shutdown(void);
	virtual void DestroyResources(void);
	virtual void Update(double time, float elapsedTime);
	virtual void Serilize(CDataArchiver* archiver);
	virtual void ListenToMessages(char* message);
	void Render(CameraInfo& cam);
	void RenderShadowMap( CLightManager* pLightMgr, CameraInfo& cam );

	///<summary>Used for setting the lighing and other per frame stuff.</summary>
	CSkinEffect* GetEffect();

	bool AddObject(PositionData data,const char* filename,const wchar_t* textureDir,char* clipName);
	// Same thing but with the index returned as well and a method callback
	bool AddObject(PositionData data,const char* filename,const wchar_t* textureDir,char* clipName,CPlayer* callback, BulletData pd);

	void OnResized(const DXGI_SURFACE_DESC* sd, ID3D11Device* device);

	PositionData GetPlayerCoordinates();

	// Be aware that with this function the user is in charge of all destruction.
	void AddObjectToRender(CStaticObject* object)
	{
		m_ObjectsToRender.push_back( object );
	}

private:
	typedef std::vector<CAnimatedObject*> AnimatedObjectList;
	AnimatedObjectList mObjectList;
	int m_iSelectedObject;
	int m_iIndex;

	CSkinEffect* mEffect;
	ID3D11InputLayout* mInputLayout;

	TextureMgr m_TextureMgr;
	CGameObjMgr* m_ptObjCtrlMgr;
	//TODO:
	// Put this someplace more convenient this was the easiest place to put this at the time.
	std::vector<CStaticObject*> m_ObjectsToRender;
};

