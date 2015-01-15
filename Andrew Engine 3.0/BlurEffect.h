#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "manager.h"
#include "StaticObject.h"

class CBlurRenderEffect :
	public CManager
{
public:
	struct Vertex
	{
		Vertex(){}
		Vertex(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT2& uv)
			: Position(p), Normal(n), TangentU(t), TexC(uv){}
		Vertex(
			float px, float py, float pz, 
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v)
			: Position(px,py,pz), Normal(nx,ny,nz),
			  TangentU(tx, ty, tz), TexC(u,v){}

		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT3 TangentU;
		XMFLOAT2 TexC;
	};
	struct Vertex32
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};
	struct QuadData
	{
		std::vector<Vertex> Vertices;
		std::vector<UINT> Indices;
	};
	
	

	CBlurRenderEffect(HINSTANCE hInstance,HWND mainWindow);
	~CBlurRenderEffect(void);

	bool Init(CEventManager* eventManager, CHashedString messageToWatch);
	bool InitializeResources(ID3D11Device* device,const DXGI_SURFACE_DESC* sd);
	void Shutdown(void);
	void DestroyResources(void);
	void OnDialogQuit(void);
	void Serilize(CDataArchiver* archiver);
	void OnResized(const DXGI_SURFACE_DESC* sd, ID3D11Device* device);
	void Render(ID3D11DeviceContext* context, CEffectManager* effectManager);

	void InitEffect(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format);
	void BuildOffscreenViews(ID3D11Device* device);

	void BlurInPlace(ID3D11DeviceContext* context,CEffectManager* effectManager);

	bool HasMultipleRenderingPasses()
	{
		return m_bHasTwoRenderPasses;
	}
	void SetHeight(UINT height) { m_uiHeight = height; }
	void SetWidth(UINT width) { m_uiWidth = width; }

	void DrawScreenQuad(ID3D11DeviceContext* context,CEffectManager* effectManager);
protected:
	BOOL CreateDlgWindow(void);

private:
	void SetGaussianWeights(float sigma);
	void CreateFullscreenQuad(QuadData& meshData);

private:
	UINT m_uiWidth;
	UINT m_uiHeight;
	DXGI_FORMAT m_Format;

	ID3D11Buffer* m_pIB;
	ID3D11Buffer* m_pVB;

	bool m_bHasTwoRenderPasses;

	float* m_afWeights;
	int m_iBlurCount;

	ID3D11ShaderResourceView* m_pBlurredOutputTexSRV;
	ID3D11UnorderedAccessView* m_pBlurredOutputTexUAV;

	ID3D11ShaderResourceView* m_pOffscreenSRV;
	ID3D11UnorderedAccessView* m_pOffscreenUAV;
	ID3D11RenderTargetView* m_pOffscreenRTV;
	
public:
	void Prerender(ID3D11DeviceContext* context);

};

