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
	mLightingCompute = new LightingCompute();

	mDSStateDefault = NULL;
	mDSStateStencilCull = NULL;
	mDSStateStencilWrite = NULL;


	mRecompile = FALSE;

};

Renderer::~Renderer() {
	SAFE_DELETE(mCamera);
	SAFE_DELETE(mCubeLoader);
	SAFE_DELETE(mCubeMesh);
	SAFE_DELETE(mShaderManager);
	SAFE_DELETE(mFXAAShader);
	SAFE_DELETE(mLightingShader);
	SAFE_DELETE(mLightingCompute);
};

void Renderer::OnD3D11DestroyDevice()
{
	mCubeMesh->OnD3D11DestroyDevice();
	mShaderManager->OnD3D11DestroyDevice();
	mFXAAShader->OnD3D11DestroyDevice();
	mLightingShader->OnD3D11DestroyDevice();
	mLightingCompute->OnD3D11DestroyDevice();

	mProxyTexture.OnD3D11DestroyDevice();
	mGBuffer[0].OnD3D11DestroyDevice();
	mGBuffer[1].OnD3D11DestroyDevice();
	mDepthStencil.OnD3D11DestroyDevice();
	mDSV.OnD3D11DestroyDevice();
	mDSVRO.OnD3D11DestroyDevice();
	mDSSRV.OnD3D11DestroyDevice();
	mLightingCSFBSB.OnD3D11DestroyDevice();

	SAFE_RELEASE(mDSStateDefault);
	SAFE_RELEASE(mDSStateStencilCull);
	SAFE_RELEASE(mDSStateStencilWrite);
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
	mLightingCompute->OnD3D11CreateDevice(pd3dDevice);

	{
		//Default depth state
		CD3D11_DEFAULT dsDefault;
		CD3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC(dsDefault);
		pd3dDevice->CreateDepthStencilState(&desc,&mDSStateDefault);

		//Depth Test, Stencil writes
		desc.DepthEnable = TRUE;
		desc.StencilEnable = TRUE;
		//Write to the stencil buffer with reference value, if the depth test passes
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;   
		//Don't care about back face so keep at default (never write)
		pd3dDevice->CreateDepthStencilState(&desc,&mDSStateStencilWrite);

		//No Depth test or writes, stencil cull
		desc.DepthEnable = FALSE;
		desc.StencilEnable = TRUE;
		//If equal to ref value, pass
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;  
		pd3dDevice->CreateDepthStencilState(&desc,&mDSStateStencilCull);
	}

	return S_OK;
}

HRESULT Renderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	mLogger->log(L"Renderer OnD3D11ResizedSwapChain");
	this->mSurfaceDescription = *pBackBufferSurfaceDesc;
	mCamera->updateWindowDimensions();

	//Create Proxy texture and GBuffers
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

	//Create Structure Buffer for Compute Shader output
	{
		mLightingCSFBSB.mBindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		mLightingCSFBSB.mCPUAccessFlags = 0;
		mLightingCSFBSB.mElements = pBackBufferSurfaceDesc->Height * pBackBufferSurfaceDesc->Width;
		mLightingCSFBSB.mUsage = D3D11_USAGE_DEFAULT;
		mLightingCSFBSB.CreateBuffer(pd3dDevice);
	}

	//Create Depth Buffer
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
		//Create Depth Stencil View
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		::ZeroMemory (&desc, sizeof (desc));
		desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		desc.Flags = 0;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		mDSV.mDesc = desc;
		mDSV.CreateDSV(pd3dDevice,mDepthStencil);

		//Read only DSV for stencil culling
		desc.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
		mDSVRO.mDesc = desc;
		mDSVRO.CreateDSV(pd3dDevice,mDepthStencil);
	}
	{
		//Create DS Shader Resource View
		CD3D11_SHADER_RESOURCE_VIEW_DESC desc(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, 0, 1, 0, 1);
		mDSSRV.mDesc = desc;
		mDSSRV.CreateSRV(pd3dDevice,mDepthStencil);
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
		mLightingCompute->OnD3D11CreateDevice(pd3dDevice);
		mRecompile = FALSE;
	}

	ID3D11DepthStencilView* dsv = mDSV.mDSV;

	//Clear depth stencil target
	pd3dImmediateContext->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0 );

	//Set ,then draw GBuffer
	ID3D11RenderTargetView* rtvs[2] = {mGBuffer[0].mRTV,mGBuffer[1].mRTV};
	pd3dImmediateContext->OMSetRenderTargets(2, rtvs, dsv);

	//Stencil buffer writes
	pd3dImmediateContext->OMSetDepthStencilState(mDSStateStencilWrite,1);
	mDrawableManager.Draw(pd3dImmediateContext);

	//Make the runtime happy
	ID3D11RenderTargetView* rtvs2[2] = {NULL,NULL};
	pd3dImmediateContext->OMSetRenderTargets(2,rtvs2,dsv);

	//Temporary clear until I get a skybox
	float ClearColor[4] = { 0.329f, 0.608f, 0.722f, 1.0f };
	pd3dImmediateContext->ClearRenderTargetView( mProxyTexture.mRTV, ClearColor );

	ID3D11ShaderResourceView* srvs[3] = {mGBuffer[0].mSRV,mGBuffer[1].mSRV,mDSSRV.mSRV};

	//Lighting CS
	pd3dImmediateContext->OMSetDepthStencilState(mDSStateStencilCull,1);
	mLightingCompute->DrawPost(pd3dImmediateContext,srvs,&mLightingCSFBSB);

	//Do lighting
	//Stencil buffer cull with read only stencil and depth buffer
	pd3dImmediateContext->OMSetDepthStencilState(mDSStateStencilCull,1);
	pd3dImmediateContext->OMSetRenderTargets(1, &mProxyTexture.mRTV, mDSVRO.mDSV);

	mLightingShader->DrawPost(pd3dImmediateContext,srvs,mCamera);

	//FXAA into back buffer
	//Back to the default depth state
	pd3dImmediateContext->OMSetDepthStencilState(mDSStateDefault,1);
	ID3D11RenderTargetView* backBuffer = DXUTGetD3D11RenderTargetView();
	pd3dImmediateContext->OMSetRenderTargets(1, &backBuffer, NULL);
	mFXAAShader->DrawPost(pd3dImmediateContext,mProxyTexture.mSRV);
}

void Renderer::OnExit()
{
	mLogger->log(L"Renderer OnExit");
	//TODO: cleanup
	mDrawableManager.reset();
}