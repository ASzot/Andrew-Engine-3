#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "MathHelper.h"
#include <d3d11.h>
#include "Camera.h"
#include <effects.h>
#include "d3dx11effect.h"
#include "Effects.h"
#include <btBulletDynamicsCommon.h>

class CLineDrawer
{
private:
	struct LineVert
	{
		LineVert(XMFLOAT3 pos, XMFLOAT4 color)
			: Pos(pos), Color(color)
		{
		}
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};

private:
	ID3D11Buffer* m_VertexBuffer;
	Camera* m_ptCamera;
	//The line color that we will draw with.
	XMFLOAT4 m_DefaultLineColor;

	CLineEffect* m_Effect;

public:
	// This should be called during initialization where the camera will be accessable.
	CLineDrawer( Camera* ptCamera, XMFLOAT4 color);
	~CLineDrawer(void);
	void SetDrawLocation(Vec& start,Vec& end);
	void DrawLine(ID3D11DeviceContext* context);
	void DestroyResources( );
	bool InitResources(ID3D11Device* device);

	void SetDrawColor(XMFLOAT4 c)
	{
		m_DefaultLineColor = c;
	}
};

