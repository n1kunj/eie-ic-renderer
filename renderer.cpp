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
#include "Shaders/FXAAShader.h"
#include "Shaders/LightingShader.h"

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
	mLightingShader = new LightingShader();

	mRecompile = FALSE;

};

Renderer::~Renderer() {
	SAFE_DELETE(mCamera);
	SAFE_DELETE(mCubeLoader);
	SAFE_DELETE(mCubeMesh);
	SAFE_DELETE(mShaderManager);
	SAFE_DELETE(mFXAAShader);
	SAFE_DELETE(mLightingShader);
};

void Renderer::OnD3D11DestroyDevice()
{
	mCubeMesh->OnD3D11DestroyDevice();
	mShaderManager->OnD3D11DestroyDevice();
	mFXAAShader->OnD3D11DestroyDevice();
	mLightingShader->OnD3D11DestroyDevice();
	mProxyTexture.OnD3D11DestroyDevice();
	mGBuffer[0].OnD3D11DestroyDevice();
	mGBuffer[1].OnD3D11DestroyDevice();
	mDepthStencil.OnD3D11DestroyDevice();
	mDSV.OnD3D11DestroyDevice();
	mDSSRV.OnD3D11DestroyDevice();
}

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
	mLightingShader->OnD3D11CreateDevice(pd3dDevice);

	return S_OK;
}

HRESULT Renderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	mLogger->log(L"Renderer OnD3D11ResizedSwapChain");
	this->mSurfaceDescription = *pBackBufferSurfaceDesc;
	mCamera->updateWindowDimensions();

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

		mProxyTexture.mDesc = desc;
		mProxyTexture.CreateTexture(pd3dDevice);

		mGBuffer[1].mDesc = desc;
		mGBuffer[1].CreateTexture(pd3dDevice);

		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		mGBuffer[0].mDesc = desc;
		mGBuffer[0].CreateTexture(pd3dDevice);
	}

	{
		D3D11_TEXTURE2D_DESC desc;
		::ZeroMemory (&desc, sizeof (desc));
		desc.Height = pBackBufferSurfaceDesc->Height;
		desc.Width = pBackBufferSurfaceDesc->Width;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32G8X24_TYPELESS;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		mDepthStencil.mDesc = desc;
		mDepthStencil.CreateTexture(pd3dDevice);
	}
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		::ZeroMemory (&desc, sizeof (desc));
		desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		desc.Flags = 0;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		mDSV.mDesc = desc;
		mDSV.CreateDSV(pd3dDevice,mDepthStencil);
	}
	{
		CD3D11_SHADER_RESOURCE_VIEW_DESC desc(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, 0, 1, 0, 1);
		mDSSRV.mDesc = desc;
		mDSSRV.CreateSRV(pd3dDevice,mDepthStencil);

		//::ZeroMemory (&desc, sizeof (desc));

	}

	return S_OK;
}

LRESULT Renderer::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,void* pUserContext )
{
	if (uMsg == WM_KEYDOWN) {
		if (wParam == 'R') {
			mRecompile = TRUE;
		}
	}
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
	if (mRecompile) {
		mShaderManager->OnD3D11CreateDevice(pd3dDevice);
		mLightingShader->OnD3D11CreateDevice(pd3dDevice);
		mRecompile = FALSE;
	}

	// Clear render target and the depth stencil 
	float ClearColor[4] = { 0.329f, 0.608f, 0.722f, 1.0f };
	ID3D11RenderTargetView* rtv = DXUTGetD3D11RenderTargetView();

	ID3D11DepthStencilView* dsv = mDSV.mDSV;

	//Clear render targets
	//pd3dImmediateContext->ClearRenderTargetView( mProxyTexture.mRTV, ClearColor );
	//pd3dImmediateContext->ClearRenderTargetView( mGBuffer[0].mRTV, ClearColor );
	//pd3dImmediateContext->ClearRenderTargetView( mGBuffer[1].mRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0 );

	//Set ,then draw GBuffer
	ID3D11RenderTargetView* rtvs[2] = {mGBuffer[0].mRTV,mGBuffer[1].mRTV};
	pd3dImmediateContext->OMSetRenderTargets(2, rtvs, dsv);
	mDrawableManager.Draw(pd3dImmediateContext);

	//Do lighting
	pd3dImmediateContext->OMSetRenderTargets(1, &mProxyTexture.mRTV, NULL);
	pd3dImmediateContext->PSSetShaderResources( 0, 1, &mGBuffer[0].mSRV );
	pd3dImmediateContext->PSSetShaderResources( 1, 1, &mGBuffer[1].mSRV );
	pd3dImmediateContext->PSSetShaderResources( 2, 1, &mDSSRV.mSRV );
	mLightingShader->DrawPost(pd3dImmediateContext);

	//FXAA into back buffer
	pd3dImmediateContext->OMSetRenderTargets(1, &rtv, dsv);
	mFXAAShader->DrawPost(pd3dImmediateContext,mProxyTexture.mSRV);
}

void Renderer::OnExit()
{
	mLogger->log(L"Renderer OnExit");
	//TODO: cleanup
	mDrawableManager.reset();
}