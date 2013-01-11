#include "DXUT.h"
#include "Cube.h"
#include "..\Utils\ShaderTools.h"

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ConstantBuffer
{
	FLOAT Time;
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

};

CubeMesh::CubeMesh()
{

}

CubeMesh::~CubeMesh()
{

}

boolean CubeMesh::sInitialised = false;

ID3D11VertexShader* CubeMesh::sVertexShader = NULL;
ID3D11PixelShader* CubeMesh::sPixelShader = NULL;
ID3D11InputLayout* CubeMesh::sVertexLayout = NULL;
ID3D11Buffer* CubeMesh::sVertexBuffer = NULL;
ID3D11Buffer* CubeMesh::sIndexBuffer = NULL;
ID3D11Buffer* CubeMesh::sConstantBuffer = NULL;

void CubeMesh::cleanup() {
	sInitialised = false;
	SAFE_RELEASE(sVertexShader);
	SAFE_RELEASE(sPixelShader);
	SAFE_RELEASE(sVertexLayout);
	SAFE_RELEASE(sVertexBuffer);
	SAFE_RELEASE(sIndexBuffer);
	SAFE_RELEASE(sConstantBuffer);
}

HRESULT CubeMesh::init( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc )
{
	if (sInitialised) {
		return S_OK;
	}
	sInitialised = true;

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
	hr = pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &sVertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE( layout );

	// Create the input layout
	hr = pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &sVertexLayout );
	pVSBlob->Release();
	V_RETURN(hr);

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
	hr = pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &sPixelShader );
	pPSBlob->Release();
	V_RETURN(hr);

	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 0.0f, 1.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ) },
	};
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	//TODO: sizeof array
	bd.ByteWidth = sizeof( SimpleVertex ) * 8;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = vertices;
	hr = pd3dDevice->CreateBuffer( &bd, &InitData, &sVertexBuffer );
	if( FAILED( hr ) )
		return hr;

	// Create index buffer
	WORD indices[] =
	{
		3,1,0,
		2,1,3,

		0,5,4,
		1,5,0,

		3,4,7,
		0,4,3,

		1,6,5,
		2,6,1,

		2,7,6,
		3,7,2,

		6,4,5,
		7,4,6,
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( WORD ) * 36;        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = pd3dDevice->CreateBuffer( &bd, &InitData, &sIndexBuffer );
	if( FAILED( hr ) )
		return hr;

	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = pd3dDevice->CreateBuffer( &bd, NULL, &sConstantBuffer );

	if( FAILED( hr ) )
		return hr;

	// Initialize the world matrix
	mWorldViewMatrix = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( 0.0f, 1.0f, -5.0f, 0.0f );
	XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	this->mViewMatrix = XMMatrixLookAtLH( Eye, At, Up );


	// Initialize the projection matrix
	this->mProjectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV2, pSurfaceDesc->Width / (FLOAT)pSurfaceDesc->Height, 0.01f, 100.0f );

	return S_OK;
}

HRESULT CubeMesh::draw( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc )
{
	if (sInitialised == false) {
		HRESULT hr;
		V_RETURN(init(pd3dDevice,pd3dContext,pSurfaceDesc));
	}
	static float t = 0.0f;
	static DWORD dwTimeStart = 0;

	DWORD dwTimeCur = GetTickCount();
	if( dwTimeStart == 0 )
		dwTimeStart = dwTimeCur;
	t = ( dwTimeCur - dwTimeStart ) / 1000.0f;

	mWorldViewMatrix = XMMatrixRotationY( t );


	// Update variables
	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose( mWorldViewMatrix );
	cb.mView = XMMatrixTranspose( mViewMatrix );
	cb.mProjection = XMMatrixTranspose( mProjectionMatrix );
	cb.Time = t;

	// Renders a triangle
	pd3dContext->UpdateSubresource( this->sConstantBuffer, 0, NULL, &cb, 0, 0 );

	pd3dContext->IASetInputLayout( sVertexLayout );

	UINT stride = sizeof( SimpleVertex );
	UINT offset = 0;
	pd3dContext->IASetVertexBuffers( 0, 1, &sVertexBuffer, &stride, &offset );

	pd3dContext->IASetIndexBuffer( sIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
	pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	pd3dContext->VSSetShader( sVertexShader, NULL, 0 );
	pd3dContext->VSSetConstantBuffers( 0, 1, &sConstantBuffer );
	pd3dContext->PSSetShader( sPixelShader, NULL, 0 );
	pd3dContext->DrawIndexed( 36, 0, 0 );        // 36 vertices needed for 12 triangles in a triangle list

	return S_OK;
}
