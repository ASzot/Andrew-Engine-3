#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include <xnamath.h>

namespace EffectConstants
{
	enum ParticleRenderTech 
	{ 
		PRT_VOLUME_SOFT,
		PRT_VOLUME_HARD,
		PRT_BILLBOARD_ODEPTH_SOFT,
		PRT_BILLBOARD_ODEPTH,
		PRT_BILLBOARD_SOFT,
		PRT_BILLBOARD_HARD 
	};

	namespace Vertex
	{
		struct PosNormalTexTanSkinned
		{
			XMFLOAT3 Pos;
			XMFLOAT3 Normal;
			XMFLOAT2 Tex;
			XMFLOAT4 TangentU;
			XMFLOAT3 Weights;
			BYTE BoneIndices[4];
		};	

		struct PosNormalTexTan
		{
			XMFLOAT3 Pos;
			XMFLOAT3 Normal;
			XMFLOAT2 Tex;
			XMFLOAT4 TangentU;
		};
	};
};

enum RenderPass{ RenderReflectionPass, RenderNormalPass };


struct ExtraRenderingData
{
	ID3D11ShaderResourceView* pShadowMap;
	XMMATRIX ShadowTransform;
};