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

ID3D11DepthStencilState* gDepthState;

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

	mCubeMesh->OnD3D11CreateDevice(pd3dDevice);
	mShaderManager->OnD3D11CreateDevice(pd3dDevice);
	mFXAAShader->OnD3D11CreateDevice(pd3dDevice);








	return S_OK;
}

HRESULT Renderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	mLogger->log(L"Renderer OnD3D11ResizedSwapChain");
	this->mSurfaceDescription = *pBackBufferSurfaceDesc;
	mCamera->updateWindowDimensions();


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

	mProxyTexture.mDesc = desc;
	mProxyTexture.CreateTexture(pd3dDevice);

	mGBuffer[1].mDesc = desc;
	mGBuffer[1].CreateTexture(pd3dDevice);

	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	mGBuffer[0].mDesc = desc;
	mGBuffer[0].CreateTexture(pd3dDevice);
	
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

	pd3dImmediateContext->ClearRenderTargetView( mProxyTexture.mRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0 );










	pd3dImmediateContext->OMSetRenderTargets(1, &mProxyTexture.mRTV, dsv);

	mDrawableManager.Draw(pd3dImmediateContext);

	pd3dImmediateContext->OMSetRenderTargets(1, &rtv, dsv);



	mFXAAShader->DrawPost(pd3dImmediateContext,mProxyTexture.mSRV);

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
	mProxyTexture.OnD3D11DestroyDevice();
	mGBuffer[0].OnD3D11DestroyDevice();
	mGBuffer[1].OnD3D11DestroyDevice();
}