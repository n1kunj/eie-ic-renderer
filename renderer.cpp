#include "DXUT.h"
#include "Renderer.h"
#include "DXUTcamera.h"
#include "SDKmesh.h"
#include "DevConsole.h"
#include "Utils/ShaderTools.h"

class RendererImplementation {
public:
	RendererImplementation(DevConsole* devConsole) : devConsole(devConsole) {
		g_iCBVSPerObjectBind = 0;
		g_iCBPSPerObjectBind = 0;
		g_iCBPSPerFrameBind = 1;
		g_pVertexLayout11 = NULL;
		g_pVertexBuffer = NULL;
		g_pIndexBuffer = NULL;
		g_pVertexShader = NULL;
		g_pPixelShader = NULL;
		g_pSamLinear = NULL;

		g_pcbVSPerObject = NULL;
		g_pcbPSPerObject = NULL;
		g_pcbPSPerFrame = NULL;
	};
	~RendererImplementation() {
		// TODO: Destruct
	};

	void init();

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );
	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
		void* pUserContext );
	void OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext );
	void OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );

	void OnD3D11DestroyDevice();
	void OnExit();

private:
	DevConsole* devConsole;
	CModelViewerCamera          g_Camera;               // A model viewing camera
	CDXUTDirectionWidget        g_LightControl;
	D3DXMATRIXA16               g_mCenterMesh;
	float                       g_fLightScale;
	int                         g_nNumActiveLights;
	int                         g_nActiveLight;

	CDXUTSDKMesh                g_Mesh11;

	ID3D11InputLayout*          g_pVertexLayout11;
	ID3D11Buffer*               g_pVertexBuffer;
	ID3D11Buffer*               g_pIndexBuffer;
	ID3D11VertexShader*         g_pVertexShader;
	ID3D11PixelShader*          g_pPixelShader;
	ID3D11SamplerState*         g_pSamLinear;

	UINT                        g_iCBVSPerObjectBind;
	UINT                        g_iCBPSPerObjectBind;
	UINT                        g_iCBPSPerFrameBind;

	ID3D11Buffer*               g_pcbVSPerObject;
	ID3D11Buffer*               g_pcbPSPerObject;
	ID3D11Buffer*               g_pcbPSPerFrame;
};

void RendererImplementation::init()
{
	devConsole->log(L"Renderer Initialisation");
	D3DXVECTOR3 vLightDir( -1, 1, -1 );
	D3DXVec3Normalize( &vLightDir, &vLightDir );
	g_LightControl.SetLightDirection( vLightDir );
}

struct CB_VS_PER_OBJECT
{
	D3DXMATRIX m_WorldViewProj;
	D3DXMATRIX m_World;
};

struct CB_PS_PER_OBJECT
{
	D3DXVECTOR4 m_vObjectColor;
};

struct CB_PS_PER_FRAME
{
	D3DXVECTOR4 m_vLightDirAmbient;
};

HRESULT RendererImplementation::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	devConsole->log(L"Renderer OnD3D11CreateDevice");

	HRESULT hr;

	D3DXVECTOR3 vCenter( 0.25767413f, -28.503521f, 111.00689f );
	FLOAT fObjectRadius = 378.15607f;

	D3DXMatrixTranslation( &g_mCenterMesh, -vCenter.x, -vCenter.y, -vCenter.z );
	D3DXMATRIXA16 m;
	D3DXMatrixRotationY( &m, D3DX_PI );
	g_mCenterMesh *= m;
	D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
	g_mCenterMesh *= m;

	// Compile the shaders using the lowest possible profile for broadest feature level support
	ID3DBlob* pVertexShaderBuffer = NULL;
	V_RETURN( ShaderTools::CompileShaderFromFile( L"BasicHLSL11_VS.hlsl", "VSMain", "vs_4_0_level_9_1", &pVertexShaderBuffer ) );

	ID3DBlob* pPixelShaderBuffer = NULL;
	V_RETURN( ShaderTools::CompileShaderFromFile( L"BasicHLSL11_PS.hlsl", "PSMain", "ps_4_0_level_9_1", &pPixelShaderBuffer ) );

	// Create the shaders
	V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShader ) );
	DXUT_SetDebugName( g_pVertexShader, "VSMain" );
	V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
		pPixelShaderBuffer->GetBufferSize(), NULL, &g_pPixelShader ) );
	DXUT_SetDebugName( g_pPixelShader, "PSMain" );

	// Create our vertex input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	V_RETURN( pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(), &g_pVertexLayout11 ) );
	DXUT_SetDebugName( g_pVertexLayout11, "Primary" );

	SAFE_RELEASE( pVertexShaderBuffer );
	SAFE_RELEASE( pPixelShaderBuffer );

	// Load the mesh
	V_RETURN( g_Mesh11.Create( pd3dDevice, L"tiny\\tiny.sdkmesh", true ) );

	// Create a sampler state
	D3D11_SAMPLER_DESC SamDesc;
	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.MipLODBias = 0.0f;
	SamDesc.MaxAnisotropy = 1;
	SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
	SamDesc.MinLOD = 0;
	SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
	V_RETURN( pd3dDevice->CreateSamplerState( &SamDesc, &g_pSamLinear ) );
	DXUT_SetDebugName( g_pSamLinear, "Primary" );

	// Setup constant buffers
	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;

	Desc.ByteWidth = sizeof( CB_VS_PER_OBJECT );
	V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbVSPerObject ) );
	DXUT_SetDebugName( g_pcbVSPerObject, "CB_VS_PER_OBJECT" );

	Desc.ByteWidth = sizeof( CB_PS_PER_OBJECT );
	V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbPSPerObject ) );
	DXUT_SetDebugName( g_pcbPSPerObject, "CB_PS_PER_OBJECT" );

	Desc.ByteWidth = sizeof( CB_PS_PER_FRAME );
	V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &g_pcbPSPerFrame ) );
	DXUT_SetDebugName( g_pcbPSPerFrame, "CB_PS_PER_FRAME" );

	// Setup the camera's view parameters
	D3DXVECTOR3 vecEye( 0.0f, 0.0f, -100.0f );
	D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
	g_Camera.SetViewParams( &vecEye, &vecAt );
	g_Camera.SetRadius( fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 10.0f );



	return S_OK;
}

HRESULT RendererImplementation::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	devConsole->log(L"Renderer OnD3D11ResizedSwapChain");

	// Setup the camera's projection parameters
	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 2.0f, 4000.0f );
	g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	g_Camera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );

	return S_OK;
}

LRESULT RendererImplementation::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
								   void* pUserContext ) {
	g_LightControl.HandleMessages( hWnd, uMsg, wParam, lParam );

	// Pass all remaining windows messages to camera so it can respond to user input
	g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

	return 0;
}

void RendererImplementation::OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	// Update the camera's position based on user input 
	g_Camera.FrameMove( fElapsedTime );
	//TODO: this
}

void RendererImplementation::OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext )
{
	// Clear render target and the depth stencil 
	float ClearColor[4] = { 1.0f, 0.196f, 0.667f, 0.0f };

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	//Copy pasted

	HRESULT hr;

	D3DXMATRIX mWorldViewProjection;
	D3DXVECTOR3 vLightDir;
	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;

	// Get the projection & view matrix from the camera class
	mProj = *g_Camera.GetProjMatrix();
	mView = *g_Camera.GetViewMatrix();

	// Get the light direction
	vLightDir = g_LightControl.GetLightDirection();

	// Per frame cb update
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	V( pd3dImmediateContext->Map( g_pcbPSPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
	CB_PS_PER_FRAME* pPerFrame = ( CB_PS_PER_FRAME* )MappedResource.pData;
	float fAmbient = 0.1f;
	pPerFrame->m_vLightDirAmbient = D3DXVECTOR4( vLightDir.x, vLightDir.y, vLightDir.z, fAmbient );
	pd3dImmediateContext->Unmap( g_pcbPSPerFrame, 0 );

	pd3dImmediateContext->PSSetConstantBuffers( g_iCBPSPerFrameBind, 1, &g_pcbPSPerFrame );

	//Get the mesh
	//IA setup
	pd3dImmediateContext->IASetInputLayout( g_pVertexLayout11 );
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = g_Mesh11.GetVB11( 0, 0 );
	Strides[0] = ( UINT )g_Mesh11.GetVertexStride( 0, 0 );
	Offsets[0] = 0;
	pd3dImmediateContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
	pd3dImmediateContext->IASetIndexBuffer( g_Mesh11.GetIB11( 0 ), g_Mesh11.GetIBFormat11( 0 ), 0 );

	// Set the shaders
	pd3dImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	pd3dImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );

	// Set the per object constant data
	mWorld = g_mCenterMesh * *g_Camera.GetWorldMatrix();
	mProj = *g_Camera.GetProjMatrix();
	mView = *g_Camera.GetViewMatrix();

	mWorldViewProjection = mWorld * mView * mProj;

	// VS Per object
	V( pd3dImmediateContext->Map( g_pcbVSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
	CB_VS_PER_OBJECT* pVSPerObject = ( CB_VS_PER_OBJECT* )MappedResource.pData;
	D3DXMatrixTranspose( &pVSPerObject->m_WorldViewProj, &mWorldViewProjection );
	D3DXMatrixTranspose( &pVSPerObject->m_World, &mWorld );
	pd3dImmediateContext->Unmap( g_pcbVSPerObject, 0 );

	pd3dImmediateContext->VSSetConstantBuffers( g_iCBVSPerObjectBind, 1, &g_pcbVSPerObject );

	// PS Per object
	V( pd3dImmediateContext->Map( g_pcbPSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
	CB_PS_PER_OBJECT* pPSPerObject = ( CB_PS_PER_OBJECT* )MappedResource.pData;
	pPSPerObject->m_vObjectColor = D3DXVECTOR4( 1, 1, 1, 1 );
	pd3dImmediateContext->Unmap( g_pcbPSPerObject, 0 );

	pd3dImmediateContext->PSSetConstantBuffers( g_iCBPSPerObjectBind, 1, &g_pcbPSPerObject );

	//Render
	SDKMESH_SUBSET* pSubset = NULL;
	D3D11_PRIMITIVE_TOPOLOGY PrimType;

	pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSamLinear );

	for( UINT subset = 0; subset < g_Mesh11.GetNumSubsets( 0 ); ++subset )
	{
		// Get the subset
		pSubset = g_Mesh11.GetSubset( 0, subset );

		PrimType = CDXUTSDKMesh::GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
		pd3dImmediateContext->IASetPrimitiveTopology( PrimType );

		// TODO: D3D11 - material loading
		ID3D11ShaderResourceView* pDiffuseRV = g_Mesh11.GetMaterial( pSubset->MaterialID )->pDiffuseRV11;
		pd3dImmediateContext->PSSetShaderResources( 0, 1, &pDiffuseRV );

		pd3dImmediateContext->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
	}
}

void RendererImplementation::OnExit()
{
	devConsole->log(L"Renderer OnExit");
	//TODO: cleanup
}

void RendererImplementation::OnD3D11DestroyDevice()
{
	g_Mesh11.Destroy();

	SAFE_RELEASE( g_pVertexLayout11 );
	SAFE_RELEASE( g_pVertexBuffer );
	SAFE_RELEASE( g_pIndexBuffer );
	SAFE_RELEASE( g_pVertexShader );
	SAFE_RELEASE( g_pPixelShader );
	SAFE_RELEASE( g_pSamLinear );

	SAFE_RELEASE( g_pcbVSPerObject );
	SAFE_RELEASE( g_pcbPSPerObject );
	SAFE_RELEASE( g_pcbPSPerFrame );
}

//Public interface

Renderer::Renderer(DevConsole* devConsole) {
	_impl = new RendererImplementation(devConsole);
}

Renderer::~Renderer() {
	SAFE_DELETE(_impl);
}

HRESULT Renderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice) {
	return _impl->OnD3D11CreateDevice(pd3dDevice);
}

HRESULT Renderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc ) {
	return _impl->OnD3D11ResizedSwapChain(pd3dDevice,pBackBufferSurfaceDesc);
}

void Renderer::OnExit() {
	_impl->OnExit();
}

void Renderer::init()
{
	_impl->init();
}

void Renderer::OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	_impl->OnFrameMove(fTime,fElapsedTime,pUserContext);
}

LRESULT Renderer::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
						 void* pUserContext ) {
	return _impl->MsgProc(hWnd,uMsg,wParam,lParam,pbNoFurtherProcessing,pUserContext);
}

void Renderer::OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext )
{
	_impl->OnD3D11FrameRender(pd3dDevice,pd3dImmediateContext,fTime,fElapsedTime,pUserContext);	
}

void Renderer::OnD3D11DestroyDevice() {
	_impl->OnD3D11DestroyDevice();
}
