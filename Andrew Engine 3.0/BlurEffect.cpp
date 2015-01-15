//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "BlurEffect.h"


CBlurRenderEffect::CBlurRenderEffect(HINSTANCE hInstance,HWND mainWindow)
	: CManager(hInstance,mainWindow,eEffectEventType,new WCF::BlurPropDialog()), m_bHasTwoRenderPasses(true),
	m_pBlurredOutputTexSRV(0), m_pBlurredOutputTexUAV(0), m_pOffscreenSRV(0), m_pOffscreenUAV(0), m_pOffscreenRTV(0),
	m_pVB(0), m_pIB(0)
{
	m_iBlurCount = 1;
}


CBlurRenderEffect::~CBlurRenderEffect(void)
{
}


bool CBlurRenderEffect::Init(CEventManager* eventManager, CHashedString messageToWatch)
{
	CManager::Init(eventManager,messageToWatch);
	return true;
}


bool CBlurRenderEffect::InitializeResources(ID3D11Device* device,const DXGI_SURFACE_DESC* sd)
{
	m_uiWidth = sd->Width;
	m_uiHeight = sd->Height;


	QuadData data;

	CreateFullscreenQuad(data);
	std::vector<Vertex32> vertices(data.Vertices.size());
	for(int i = 0; i < data.Vertices.size(); ++i)
	{
		vertices[i].Pos = data.Vertices[i].Position;
		vertices[i].Normal = data.Vertices[i].Normal;
		vertices[i].Tex = data.Vertices[i].TexC;
	}
	
	D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex32) * vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HRESULT hr = device->CreateBuffer(&vbd,&vinitData,&m_pVB);
	Checkh(hr);


	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * data.Indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &data.Indices[0];
	hr = device->CreateBuffer(&ibd,&iinitData,&m_pIB);
	Checkh(hr);

	BuildOffscreenViews(device);

	return true;
}


void CBlurRenderEffect::Shutdown(void)
{
	CManager::Shutdown();
}


void CBlurRenderEffect::DestroyResources(void)
{
	ReleaseCOM(m_pVB);
	ReleaseCOM(m_pIB);
	ReleaseCOM(m_pOffscreenSRV);
	ReleaseCOM(m_pOffscreenUAV);
	ReleaseCOM(m_pOffscreenRTV);
	ReleaseCOM(m_pBlurredOutputTexSRV);
	ReleaseCOM(m_pBlurredOutputTexUAV);
}


void CBlurRenderEffect::OnDialogQuit(void)
{
}


void CBlurRenderEffect::Serilize(CDataArchiver* archiver)
{
}


void CBlurRenderEffect::OnResized(const DXGI_SURFACE_DESC* sd, ID3D11Device* device)
{
}


void CBlurRenderEffect::Render(ID3D11DeviceContext* context, CEffectManager* effectManager)
{
	BlurInPlace(context,effectManager);
}


BOOL CBlurRenderEffect::CreateDlgWindow(void)
{
	return TRUE;
}

void CBlurRenderEffect::InitEffect(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format)
{
	ReleaseCOM(m_pBlurredOutputTexSRV);
	ReleaseCOM(m_pBlurredOutputTexUAV);

	m_uiWidth = width;
	m_uiHeight = height;
	m_Format = format;

	HRESULT hr;

	D3D11_TEXTURE2D_DESC blurredTexDesc;
	blurredTexDesc.Width     = width;
	blurredTexDesc.Height    = height;
    blurredTexDesc.MipLevels = 1;
    blurredTexDesc.ArraySize = 1;
	blurredTexDesc.Format    = format;
	blurredTexDesc.SampleDesc.Count   = 1;
	blurredTexDesc.SampleDesc.Quality = 0;
    blurredTexDesc.Usage     = D3D11_USAGE_DEFAULT;
    blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    blurredTexDesc.CPUAccessFlags = 0;
    blurredTexDesc.MiscFlags      = 0;

	ID3D11Texture2D* blurredTex = 0;
	hr = device->CreateTexture2D(&blurredTexDesc, 0, &blurredTex);
	Checkhr(hr,"Couldn't create blurred tex");

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(blurredTex, &srvDesc, &m_pBlurredOutputTexSRV);
	Checkhr(hr,"Couldn't create blurred output texture shader resource view");

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	hr = device->CreateUnorderedAccessView(blurredTex, &uavDesc, &m_pBlurredOutputTexUAV);
	Checkhr(hr,"Couldn't create the blurred output texture unordered access view");


	// Views save a reference to the texture so we can release our reference.
	ReleaseCOM(blurredTex);
}

void CBlurRenderEffect::SetGaussianWeights(float sigma)
{
	float d = 2.0f*sigma*sigma;

	float weights[9];
	float sum = 0.0f;
	for(int i = 0; i < 8; ++i)
	{
		float x = (float)i;
		weights[i] = expf(-x*x/d);

		sum += weights[i];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for(int i = 0; i < 8; ++i)
	{
		weights[i] /= sum;
	}
	m_afWeights = weights;
}

void CBlurRenderEffect::BuildOffscreenViews(ID3D11Device* device)
{
	ReleaseCOM(m_pOffscreenSRV);
	ReleaseCOM(m_pOffscreenRTV);
	ReleaseCOM(m_pOffscreenUAV);

	D3D11_TEXTURE2D_DESC texDesc;
	
	texDesc.Width     = m_uiWidth;
	texDesc.Height    = m_uiHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count   = 1;  
	texDesc.SampleDesc.Quality = 0;  
	texDesc.Usage          = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags      = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0; 
	texDesc.MiscFlags      = 0;
	
	HRESULT hr;

	ID3D11Texture2D* offscreenTex = 0;
	hr = device->CreateTexture2D(&texDesc, 0, &offscreenTex);
	Checkhr(hr,"Couldn't create the offscreenTex.");

	// Null description means to create a view to all mipmap levels using 
	// the format the texture was created with.
	hr = device->CreateShaderResourceView(offscreenTex, 0, &m_pOffscreenSRV);
	Checkhr(hr,"Couldn't create the offscreen shader resource view.");
	hr = device->CreateRenderTargetView(offscreenTex, 0, &m_pOffscreenRTV);
	Checkhr(hr,"Couldn't create the offscreen render target view.");
	hr = device->CreateUnorderedAccessView(offscreenTex, 0, &m_pOffscreenUAV);
	Checkhr(hr,"Couldn't create the unordered access view.");

	// View saves a reference to the texture so we can release our reference.
	ReleaseCOM(offscreenTex);
}

void CBlurRenderEffect::BlurInPlace(ID3D11DeviceContext* dc,CEffectManager* effectManager)
{
	for(int i = 0; i < m_iBlurCount; ++i)
	{
		// HORIZONTAL blur pass.
		D3DX11_TECHNIQUE_DESC techDesc;
		ID3DX11EffectTechnique* renderTech = effectManager->BlurEffectAccess.GetTechnique(true);
		renderTech->GetDesc(&techDesc);
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			if(!effectManager->BlurEffectAccess.SetInputMap(m_pOffscreenSRV))
				FatalError("Couldn't set offscreen srv");
			if  ( !effectManager->BlurEffectAccess.SetOutputMap(m_pBlurredOutputTexUAV) )
				FatalError( "Couldn't set output map!" );
			renderTech->GetPassByIndex(p)->Apply(0, dc);

			// How many groups do we need to dispatch to cover a row of pixels, where each
			// group covers 256 pixels (the 256 is defined in the ComputeShader).
			UINT numGroupsX = (UINT)ceilf(m_uiWidth / 256.0f);
			dc->Dispatch(numGroupsX, m_uiHeight, 1);
		}
	
		// Unbind the input texture from the CS for good housekeeping.
		ID3D11ShaderResourceView* nullSRV[1] = { 0 };
		dc->CSSetShaderResources( 0, 1, nullSRV );

		// Unbind output from compute shader (we are going to use this output as an input in the next pass, 
		// and a resource cannot be both an output and input at the same time.
		ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
		dc->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );
	
		// VERTICAL blur pass.
		renderTech = effectManager->BlurEffectAccess.GetTechnique(false);
		renderTech->GetDesc(&techDesc);
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			effectManager->BlurEffectAccess.SetInputMap(m_pBlurredOutputTexSRV);
			effectManager->BlurEffectAccess.SetOutputMap(m_pOffscreenUAV);
			renderTech->GetPassByIndex(p)->Apply(0, dc);

			// How many groups do we need to dispatch to cover a column of pixels, where each
			// group covers 256 pixels  (the 256 is defined in the ComputeShader).
			UINT numGroupsY = (UINT)ceilf(m_uiHeight / 256.0f);
			dc->Dispatch(m_uiWidth, numGroupsY, 1);
		}
	
		dc->CSSetShaderResources( 0, 1, nullSRV );
		dc->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );
	}

	// Disable compute shader.
	dc->CSSetShader(0, 0, 0);
}


void CBlurRenderEffect::Prerender(ID3D11DeviceContext* context)
{
	ID3D11RenderTargetView* renderTargets[1] = { m_pOffscreenRTV };
	context->OMSetRenderTargets( 1,renderTargets, DXUTGetD3D11DepthStencilView() );

	float colors[4] = { 1.0f,1.0f,1.0f, 1.0f };
	context->ClearRenderTargetView( m_pOffscreenRTV, colors );
	context->ClearDepthStencilView( DXUTGetD3D11DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f,0 );
}

void CBlurRenderEffect::DrawScreenQuad(ID3D11DeviceContext* context,CEffectManager* effectManager)
{
	effectManager->SetInputLayout( InputLayoutType::PosNormTex );
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	UINT stride = sizeof(Vertex32);
	UINT offset = 0;

	ID3DX11EffectTechnique* renderTech = effectManager->GetScreenQuadTechnique();
	D3DX11_TECHNIQUE_DESC techDesc;

	renderTech->GetDesc(&techDesc);
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		context->IASetVertexBuffers(0,1,&m_pVB,&stride,&offset);
		context->IASetIndexBuffer(m_pIB,DXGI_FORMAT_R32_UINT,0);
		// Calling this will disable all lights.
		if(!effectManager->SetDrawFullScreenQuad(m_pBlurredOutputTexSRV))
			FatalError("Couldn't set the draw full screen quad shader parameters");
		renderTech->GetPassByIndex(p)->Apply(0,context);
		context->DrawIndexed(6,0,0);
	}
}


void CBlurRenderEffect::CreateFullscreenQuad(QuadData& meshData)
{
	meshData.Vertices.resize(4);
	meshData.Indices.resize(6);

	// Position coordinates specified in NDC space.
	meshData.Vertices[0] = Vertex(
		-1.0f, -1.0f, 0.0f, 
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.Vertices[1] = Vertex(
		-1.0f, +1.0f, 0.0f, 
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	meshData.Vertices[2] = Vertex(
		+1.0f, +1.0f, 0.0f, 
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	meshData.Vertices[3] = Vertex(
		+1.0f, -1.0f, 0.0f, 
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	meshData.Indices[0] = 0;
	meshData.Indices[1] = 1;
	meshData.Indices[2] = 2;

	meshData.Indices[3] = 0;
	meshData.Indices[4] = 2;
	meshData.Indices[5] = 3;
}
