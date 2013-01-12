#include "DXUT.h"
#include "Cube.h"

CubeMesh::CubeMesh()
{

}

CubeMesh::~CubeMesh()
{
	
}

HRESULT CubeMesh::OnD3D11CreateDevice( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc )
{
	if (mInitialised) {
		return S_OK;
	}

	HRESULT hr = S_OK;

	// Create vertex buffer
	Vertex vertices[] =
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
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = vertices;
	V_RETURN(pd3dDevice->CreateBuffer( &bd, &InitData, &mVertexBuffer ));

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
	bd.ByteWidth = sizeof( indices );
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	V_RETURN(hr = pd3dDevice->CreateBuffer( &bd, &InitData, &mIndexBuffer ));

	// Initialize the world matrix
	mWorldViewMatrix = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( 0.0f, 1.0f, -5.0f, 0.0f );
	XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	this->mViewMatrix = XMMatrixLookAtLH( Eye, At, Up );


	// Initialize the projection matrix
	this->mProjectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV2, pSurfaceDesc->Width / (FLOAT)pSurfaceDesc->Height, 0.01f, 100.0f );

	mInitialised = true;
	return S_OK;
}

HRESULT CubeMesh::draw( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, const DXGI_SURFACE_DESC* pSurfaceDesc )
{
	if (mInitialised == false) {
		HRESULT hr;
		V_RETURN(OnD3D11CreateDevice(pd3dDevice,pd3dContext,pSurfaceDesc));
	}

	static float t = 0.0f;
	static DWORD dwTimeStart = 0;

	DWORD dwTimeCur = GetTickCount();
	if( dwTimeStart == 0 )
		dwTimeStart = dwTimeCur;
	t = ( dwTimeCur - dwTimeStart ) / 1000.0f;

	mWorldViewMatrix = XMMatrixRotationY( t );

	return S_OK;
}
