#include "DXUT.h"
#include "Renderer.h"
#include "MessageProcessor.h"
#include "Meshes/CubeMeshLoader.h"
#include "Shaders/DefaultShader.h"
#include "DrawableState.h"
#include "Drawable.h"
#include "Camera.h"

#include <sstream>

class RendererImplementation {
public:
	RendererImplementation(MessageLogger* mLogger)
		: mLogger(mLogger){
		mCamera = new Camera();
		cubeLoader = new CubeMeshLoader();
		cubeMesh = new DrawableMesh(L"CubeMesh",cubeLoader);
		defaultShader = new DefaultShader();

		cubeDrawable = new Drawable(cubeMesh,defaultShader,mCamera);
		cubeDrawable->mState.mScale.x = 10.0f;
		cubeDrawable->mState.mScale.z = 10.0f;

		lightDrawable = new Drawable(cubeMesh,defaultShader,mCamera);
		lightDrawable->mState.mPosition = DirectX::XMFLOAT3(5.0f,3.0f,2.0f);
		lightDrawable->mState.mAmbientColour = DirectX::XMFLOAT3(9999999.0f,9999999.0f,9999999.0f);
		lightDrawable->mState.mScale = DirectX::XMFLOAT3(0.2f,0.2f,0.2f);

#ifdef DEBUG
		mRecompile = FALSE;
#endif // DEBUG

	};
	~RendererImplementation() {
		SAFE_DELETE(mCamera);
		SAFE_DELETE(cubeLoader);
		SAFE_DELETE(cubeMesh);
		SAFE_DELETE(defaultShader);
		SAFE_DELETE(cubeDrawable);
		SAFE_DELETE(lightDrawable);
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
	MessageLogger* mLogger;
	DrawableMesh* cubeMesh;
	DrawableShader* defaultShader;
	CubeMeshLoader* cubeLoader;
	Drawable* cubeDrawable;
	Drawable* lightDrawable;
	Camera* mCamera;
#ifdef DEBUG
	boolean mRecompile;
#endif // DEBUG

};

void RendererImplementation::init()
{
	mLogger->log(L"Renderer Initialisation");
}


HRESULT RendererImplementation::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	mLogger->log(L"Renderer OnD3D11CreateDevice");
	defaultShader->OnD3D11CreateDevice(pd3dDevice);
	cubeMesh->OnD3D11CreateDevice(pd3dDevice);
	return S_OK;
}

HRESULT RendererImplementation::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	mLogger->log(L"Renderer OnD3D11ResizedSwapChain");
	this->surfaceDescription = *pBackBufferSurfaceDesc;
	mCamera->updateWindowDimensions();
	return S_OK;
}

LRESULT RendererImplementation::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,void* pUserContext )
{
#ifdef DEBUG
	if (uMsg == WM_KEYDOWN) {
		if (wParam == 'R') {
			mRecompile = TRUE;
		}
	}
#endif //DEBUG
	mCamera->MsgProc(hWnd,uMsg,wParam,lParam,pbNoFurtherProcessing,pUserContext);
	if (*pbNoFurtherProcessing) {
		return 0;
	}
	return 0;
}

void RendererImplementation::OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	mCamera->update(surfaceDescription);
}

void RendererImplementation::OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext )
{

#ifdef DEBUG
	if (mRecompile) {
		defaultShader->OnD3D11CreateDevice(pd3dDevice);
		mRecompile = FALSE;
	}
#endif // DEBUG



	// Clear render target and the depth stencil 
	float ClearColor[4] = { 0.329f, 0.608f, 0.722f, 1.0f };

	ID3D11RenderTargetView* rtv = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* dsv = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearRenderTargetView( rtv, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH, 1.0, 0 );

	//cubeDrawable->mState.mRotation.x = (FLOAT) fTime;
	//cubeDrawable->mState.mRotation.y = (FLOAT) fTime;
	//cubeDrawable->mState.mDirty = TRUE;

	lightDrawable->Draw(pd3dImmediateContext);
	
	cubeDrawable->Draw(pd3dImmediateContext);

}

void RendererImplementation::OnExit()
{
	mLogger->log(L"Renderer OnExit");
	//TODO: cleanup
}

void RendererImplementation::OnD3D11DestroyDevice()
{
	cubeMesh->OnD3D11DestroyDevice();
	defaultShader->OnD3D11DestroyDevice();
}

//Public interface

Renderer::Renderer(MessageLogger* logger) {
	_impl = new RendererImplementation(logger);
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
