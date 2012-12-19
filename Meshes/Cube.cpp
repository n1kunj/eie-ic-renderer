#include "DXUT.h"
#include "Cube.h"
#include <xnamath.h>
#include "..\Utils\ShaderTools.h"

struct SimpleVertex
{
	XMFLOAT3 Pos;  // Position
};

CubeMesh::CubeMesh()
{

}

CubeMesh::~CubeMesh()
{

}

boolean CubeMesh::dirty = false;

ID3D11VertexShader* CubeMesh::vertexShader = NULL;
ID3D11PixelShader* CubeMesh::pixelShader = NULL;
ID3D11InputLayout* CubeMesh::vertexLayout = NULL;
ID3D11Buffer* CubeMesh::vertexBuffer = NULL;

void CubeMesh::cleanup() {
	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);
	SAFE_RELEASE(vertexLayout);
	SAFE_RELEASE(vertexBuffer);
}

HRESULT CubeMesh::init( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext)
{
	if (dirty) {
		return S_OK;
	}
	dirty = true;

	HRESULT hr = S_OK;

	// Compile the vertex shader
	ID3DBlob* pVSBlob = NULL;
	hr = ShaderTools::CompileShaderFromFile( L"Shaders\\Tutorial03.fx", "VS", "vs_4_0", &pVSBlob );
	if( FAILED( hr ) )
	{
		MessageBox( NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		return hr;
	}

	// Create the vertex shader
	hr = pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
	};
	UINT numElements = ARRAYSIZE( layout );

	// Create the input layout
	hr = pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &vertexLayout );
	pVSBlob->Release();
	if( FAILED( hr ) )
		return hr;

	// Set the input layout
	pd3dImmediateContext->IASetInputLayout( vertexLayout );

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = ShaderTools::CompileShaderFromFile( L"Shaders\\Tutorial03.fx", "PS", "ps_4_0", &pPSBlob );
	if( FAILED( hr ) )
	{
		MessageBox( NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		return hr;
	}

	// Create the pixel shader
	hr = pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pixelShader );
	pPSBlob->Release();
	if( FAILED( hr ) )
		return hr;

	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		XMFLOAT3( 0.0f, 0.5f, 0.5f ),
		XMFLOAT3( 0.5f, -0.5f, 0.5f ),
		XMFLOAT3( -0.5f, -0.5f, 0.5f ),
	};
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( SimpleVertex ) * 3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = vertices;
	hr = pd3dDevice->CreateBuffer( &bd, &InitData, &vertexBuffer );
	if( FAILED( hr ) )
		return hr;
	// Set vertex buffer
	UINT stride = sizeof( SimpleVertex );
	UINT offset = 0;
	pd3dImmediateContext->IASetVertexBuffers( 0, 1, &vertexBuffer, &stride, &offset );

	// Set primitive topology
	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}

void CubeMesh::draw(ID3D11DeviceContext* pd3dImmediateContext)
{
	// Render a triangle
	pd3dImmediateContext->VSSetShader( vertexShader, NULL, 0 );
	pd3dImmediateContext->PSSetShader( pixelShader, NULL, 0 );
	pd3dImmediateContext->Draw( 3, 0 );
}
