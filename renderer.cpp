#include "DXUT.h"
#include "Renderer.h"
#include "DevConsole.h"
#include "Meshes/Cube.h"
#include "Shaders/DefaultShader.h"

class RendererImplementation {
public:
	RendererImplementation(DevConsole* devConsole) : devConsole(devConsole) {
		cube = new CubeMesh();
		defaultShader = new DefaultShader();
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

	DXGI_SURFACE_DESC surfaceDescription;
	DevConsole* devConsole;
	DrawableMesh* cube;
	DrawableShaderInterface* defaultShader;

};

void RendererImplementation::init()
{
	devConsole->log(L"Renderer Initialisation");
}


HRESULT RendererImplementation::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	devConsole->log(L"Renderer OnD3D11CreateDevice");
	defaultShader->OnD3D11CreateDevice(pd3dDevice);
	return S_OK;
}

HRESULT RendererImplementation::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	devConsole->log(L"Renderer OnD3D11ResizedSwapChain");
	this->surfaceDescription = *pBackBufferSurfaceDesc;
	return S_OK;
}

LRESULT RendererImplementation::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
								   void* pUserContext ) {
	return 0;
}

void RendererImplementation::OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
}

void RendererImplementation::OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext )
{
	// Clear render target and the depth stencil 
	float ClearColor[4] = { 1.0f, 0.196f, 0.667f, 0.0f };

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	cube->draw(pd3dDevice,pd3dImmediateContext,&surfaceDescription);
	defaultShader->DrawMesh(pd3dImmediateContext,cube);

}

void RendererImplementation::OnExit()
{
	devConsole->log(L"Renderer OnExit");
	//TODO: cleanup
}

void RendererImplementation::OnD3D11DestroyDevice()
{
	cube->OnD3D11DestroyDevice();
	defaultShader->OnD3D11DestroyDevice();
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
