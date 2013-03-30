#include "DXUT.h"
#include "Renderer.h"
#include "MessageProcessor.h"
#include "Meshes/CubeMeshLoader.h"
#include "Shaders/DefaultShader.h"
#include "Shaders/GBufferShader.h"
#include "DrawableState.h"
#include "Drawable.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "Shaders\FXAAShader.h"

#include <sstream>

ID3D11Texture2D*			g_pProxyTexture = NULL;
ID3D11ShaderResourceView*	g_pProxyTextureSRV = NULL;
ID3D11Texture2D*			g_pCopyResolveTexture = NULL;
ID3D11ShaderResourceView*	g_pCopyResolveTextureSRV = NULL;
ID3D11RenderTargetView*     g_pProxyTextureRTV = NULL;

void FxaaIntegrateResource(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);

Renderer::Renderer(MessageLogger* mLogger) : mLogger(mLogger), mDrawableManager()
{
	mCamera = new Camera();
	mCubeLoader = new CubeMeshLoader();
	mCubeMesh = new DrawableMesh(L"CubeMesh",mCubeLoader);
	mShaderManager = new ShaderManager(mLogger);
	mShaderManager->addShader(new DefaultShader());
	mShaderManager->addShader(new GBufferShader());
	mFXAAShader = new FXAAShader();

#ifdef DEBUG
	mRecompile = FALSE;
#endif // DEBUG

};

Renderer::~Renderer() {
	SAFE_DELETE(mCamera);
	SAFE_DELETE(mCubeLoader);
	SAFE_DELETE(mCubeMesh);
	SAFE_DELETE(mShaderManager);
	SAFE_DELETE(mFXAAShader);
};

void Renderer::init()
{
	mLogger->log(L"Renderer Initialisation");
}


HRESULT Renderer::OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc  )
{
	mLogger->log(L"Renderer OnD3D11CreateDevice");

	this->mSurfaceDescription = *pBackBufferSurfaceDesc;
	mCamera->updateWindowDimensions();

	mCubeMesh->OnD3D11CreateDevice(pd3dDevice);
	mShaderManager->OnD3D11CreateDevice(pd3dDevice);
	mFXAAShader->OnD3D11CreateDevice(pd3dDevice);

	FxaaIntegrateResource(pd3dDevice, pBackBufferSurfaceDesc);
	return S_OK;
}

HRESULT Renderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	mLogger->log(L"Renderer OnD3D11ResizedSwapChain");
	this->mSurfaceDescription = *pBackBufferSurfaceDesc;
	mCamera->updateWindowDimensions();

	SAFE_RELEASE( g_pProxyTexture );
	SAFE_RELEASE( g_pProxyTextureSRV );
	SAFE_RELEASE( g_pProxyTextureRTV );
	SAFE_RELEASE( g_pCopyResolveTexture );
	SAFE_RELEASE( g_pCopyResolveTextureSRV );
	FxaaIntegrateResource(pd3dDevice, pBackBufferSurfaceDesc);

	return S_OK;
}

LRESULT Renderer::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,void* pUserContext )
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

void Renderer::OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	mCamera->update(mSurfaceDescription);
}

void Renderer::OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext )
{

#ifdef DEBUG
	if (mRecompile) {
		mShaderManager->OnD3D11CreateDevice(pd3dDevice);
		mRecompile = FALSE;
	}
#endif // DEBUG

	// Clear render target and the depth stencil 
	float ClearColor[4] = { 0.329f, 0.608f, 0.722f, 1.0f };

	ID3D11RenderTargetView* rtv = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* dsv = DXUTGetD3D11DepthStencilView();
	//pd3dImmediateContext->ClearRenderTargetView( rtv, ClearColor );

	pd3dImmediateContext->ClearRenderTargetView( g_pProxyTextureRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH, 1.0, 0 );

	ID3D11RenderTargetView* pRTVs[1] = { g_pProxyTextureRTV};

	pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, dsv);

	mDrawableManager.Draw(pd3dImmediateContext);

	mFXAAShader->DrawPost(pd3dImmediateContext,g_pProxyTextureSRV,rtv);
}

void Renderer::OnExit()
{
	mLogger->log(L"Renderer OnExit");
	//TODO: cleanup
	mDrawableManager.reset();
}

void Renderer::OnD3D11DestroyDevice()
{
	mCubeMesh->OnD3D11DestroyDevice();
	mShaderManager->OnD3D11DestroyDevice();
	mFXAAShader->OnD3D11DestroyDevice();

	SAFE_RELEASE( g_pProxyTexture );
	SAFE_RELEASE( g_pProxyTextureSRV );
	SAFE_RELEASE( g_pProxyTextureRTV );
	SAFE_RELEASE( g_pCopyResolveTexture );
	SAFE_RELEASE( g_pCopyResolveTextureSRV );
}

void FxaaIntegrateResource(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	D3D11_TEXTURE2D_DESC desc;
	::ZeroMemory (&desc, sizeof (desc));
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Height = pBackBufferSurfaceDesc->Height;
	desc.Width = pBackBufferSurfaceDesc->Width;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = pBackBufferSurfaceDesc->SampleDesc.Count;
	desc.SampleDesc.Quality = pBackBufferSurfaceDesc->SampleDesc.Quality;
	desc.MipLevels = 1;
	pd3dDevice->CreateTexture2D( &desc, 0, &g_pProxyTexture );
	pd3dDevice->CreateRenderTargetView( g_pProxyTexture, 0, &g_pProxyTextureRTV );
	pd3dDevice->CreateShaderResourceView( g_pProxyTexture, 0, &g_pProxyTextureSRV);
}