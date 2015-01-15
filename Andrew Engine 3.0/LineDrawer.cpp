//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "LineDrawer.h"


CLineDrawer::CLineDrawer(Camera* ptCamera, XMFLOAT4 color)
{
	m_ptCamera = ptCamera;
	m_DefaultLineColor = color;


}

bool CLineDrawer::InitResources( ID3D11Device* device )
{
	D3D11_BUFFER_DESC bd;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// Since a line has two verticies.
	bd.ByteWidth = sizeof(LineVert) * 2;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DYNAMIC;


	HRESULT hr = device->CreateBuffer( &bd, 0, &m_VertexBuffer );
	Checkhr(hr,"Couldn't create the vertex buffer!/nCLineDrawer::CLineDrawer(ID3D11Device* device, Camera* ptCamera, XMFLOAT4 color)");

	m_Effect = new CLineEffect(device,L"Line.fx");
	assert(m_Effect);

	return true;
}


CLineDrawer::~CLineDrawer(void)
{
}

void CLineDrawer::SetDrawLocation(Vec& start, Vec& end)
{
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT hr = DXUTGetD3D11DeviceContext()->Map( m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms );
	Checkhr( hr, "Couldn't map to the vertex buffer!/nvoid CLineDrawer::SetDrawLocation(Vec& start, Vec& end)" );
	
	LineVert* v = reinterpret_cast<LineVert*>(ms.pData);

	v[0].Color = m_DefaultLineColor;
	v[0].Pos = start.ToFloat3();
	v[1].Color = m_DefaultLineColor;
	v[1].Pos = end.ToFloat3();

	DXUTGetD3D11DeviceContext()->Unmap( m_VertexBuffer, 0 );
}

void CLineDrawer::DestroyResources()
{
	ReleaseCOM(m_VertexBuffer);
	if( m_Effect)
	{
		m_Effect->DestoryResources();
		delete m_Effect;
		m_Effect = 0;
	}
}

void CLineDrawer::DrawLine(ID3D11DeviceContext* context)
{
	// Set all of the per draw call stuff.
	m_Effect->SetWorld( XMMatrixIdentity() );
	m_Effect->SetProj( m_ptCamera->Proj() );
	m_Effect->SetView( m_ptCamera->View() );

	context->IASetInputLayout( m_Effect->GetInputLayout() );

	UINT stride = sizeof( LineVert );
	UINT offset = 0;

	// Since we are rendering simple lines that only have 2 verts.
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	context->IASetVertexBuffers( 0, 1, &m_VertexBuffer, &stride, &offset );

	ID3DX11EffectTechnique* effectTech = m_Effect->GetTech();
	D3DX11_TECHNIQUE_DESC techDesc;
	effectTech->GetDesc( &techDesc );
	for ( UINT p = 0; p < techDesc.Passes; ++p )
	{
		effectTech->GetPassByIndex( p )->Apply( 0, context );
		context->Draw( 2, 0 );
	}

	context->OMSetDepthStencilState(0,0);
	context->OMSetBlendState(0,0,0);
}
