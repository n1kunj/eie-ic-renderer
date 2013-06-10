#include "DXUT.h"
#include "Renderer.h"
#include "MessageProcessor.h"
#include "Meshes/CubeMeshLoader.h"
#include "Meshes/PlaneLoader.h"
#include "Shaders/GBufferShader.h"
#include "Shaders/DistantGBufferShader.h"
#include "DrawableState.h"
#include "Drawable.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include "Shaders/FXAAShader.h"
#include "Shaders/LightingShader.h"
#include "Procedural/Generator.h"

#include <sstream>

Renderer::Renderer(MessageLogger* mLogger) : mLogger(mLogger), mDrawableManager()
{
	mCamera = new Camera();
	mMeshManager = new MeshManager();
	mMeshManager->addMesh(new DrawableMesh(L"CubeMesh",new CubeMeshLoader()));
	mMeshManager->addMesh(new DrawableMesh(L"Plane2",new PlaneLoader(3)));
	mMeshManager->addMesh(new DrawableMesh(L"Plane4",new PlaneLoader(5)));
	mMeshManager->addMesh(new DrawableMesh(L"Plane8",new PlaneLoader(9)));
	mMeshManager->addMesh(new DrawableMesh(L"Plane16",new PlaneLoader(17)));
	mMeshManager->addMesh(new DrawableMesh(L"Plane32",new PlaneLoader(33)));
	mMeshManager->addMesh(new DrawableMesh(L"Plane64",new PlaneLoader(65)));
	mMeshManager->addMesh(new DrawableMesh(L"Plane128",new PlaneLoader(129)));
	mMeshManager->addMesh(new DrawableMesh(L"Plane256",new PlaneLoader(257)));
	mShaderManager = new ShaderManager(mLogger);
	mShaderManager->addShader(new GBufferShader());
	mShaderManager->addShader(new DistantGBufferShader());

	mLightManager = new LightManager();
	mFXAAShader = new FXAAShader();
	mLightingShader = new LightingShader();
	mLightingCompute = new LightingCompute();

	mGenerator = new Generator(mLogger);

	mDSStateDefault = NULL;
	mDSStateStencilCull = NULL;
	mDSStateStencilWrite = NULL;

	mRasterizerStateDefault = NULL;
	mRasterizerStateWireframe = NULL;

	mRecompile = FALSE;
};

Renderer::~Renderer() {
	SAFE_DELETE(mCamera);
	SAFE_DELETE(mShaderManager);
	SAFE_DELETE(mMeshManager);
	SAFE_DELETE(mFXAAShader);
	SAFE_DELETE(mLightingShader);
	SAFE_DELETE(mLightingCompute);
	SAFE_DELETE(mGenerator);
};

void Renderer::OnD3D11DestroyDevice()
{
	mShaderManager->OnD3D11DestroyDevice();
	mMeshManager->OnD3D11DestroyDevice();
	mFXAAShader->OnD3D11DestroyDevice();
	mLightingShader->OnD3D11DestroyDevice();
	mLightingCompute->OnD3D11DestroyDevice();
	mGenerator->OnD3D11DestroyDevice();

	mProxyTexture.OnD3D11DestroyDevice();
	mGBuffer[0].OnD3D11DestroyDevice();
	mGBuffer[1].OnD3D11DestroyDevice();
	mDepthStencil.OnD3D11DestroyDevice();
	mDSV.OnD3D11DestroyDevice();
	mDSVRO.OnD3D11DestroyDevice();
	mDSSRV.OnD3D11DestroyDevice();
	mLightingCSFBSB.OnD3D11DestroyDevice();
	mLightListCSSB.OnD3D11DestroyDevice();
	mGenerator->OnD3D11DestroyDevice();

	SAFE_RELEASE(mDSStateDefault);
	SAFE_RELEASE(mDSStateStencilCull);
	SAFE_RELEASE(mDSStateStencilWrite);
	SAFE_RELEASE(mRasterizerStateDefault);
	SAFE_RELEASE(mRasterizerStateWireframe);
}

void Renderer::init()
{
	mLogger->log(L"Renderer Initialisation");

	//for (int i = 0; i < 1023; i++) {
	//	PointLight* ll = &mLightList[i];
	//	ll->ambient = 0.001f;
	//	ll->attenuationEnd = ((float)rand()/(float)RAND_MAX) * 5 + 5;
	//	ll->colour.x = ((float)rand()/(float)RAND_MAX) * 3;
	//	ll->colour.y = ((float)rand()/(float)RAND_MAX) * 3;
	//	ll->colour.z = ((float)rand()/(float)RAND_MAX) * 3;

	//	ll->x = ((double)rand()/(double)RAND_MAX) * 100 - 50;
	//	ll->y = ((double)rand()/(double)RAND_MAX) * 100 - 50;
	//	ll->z = ((double)rand()/(double)RAND_MAX) * 100 - 50;
	//	mLightManager->addLight(ll);
	//}
	//THE SUN
	PointLight* ll = &mLightList[1023];
	ll->ambient = 0.05f;
	ll->attenuationEnd = FLT_MAX/10.0f;
	ll->colour.x = 1.0f;
	ll->colour.y = 1.0f;
	ll->colour.z = 1.0f;
	ll->x = 50000000000.0f;
	ll->z = 111445532285.0f;
	ll->y = 86371600270.0f;
	//ll->x = 100000000000;
	//ll->z = 150000000000.0f;
	//ll->y = 10000000000.0f;
	mLightManager->addLight(ll);

}


HRESULT Renderer::OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc  )
{
	mLogger->log(L"Renderer OnD3D11CreateDevice");

	mShaderManager->OnD3D11CreateDevice(pd3dDevice);
	mMeshManager->OnD3D11CreateDevice(pd3dDevice);

	//Post shaders
	mFXAAShader->OnD3D11CreateDevice(pd3dDevice);
	mLightingShader->OnD3D11CreateDevice(pd3dDevice);
	mLightingCompute->OnD3D11CreateDevice(pd3dDevice);

	mGenerator->OnD3D11CreateDevice(pd3dDevice);
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

	{
		CD3D11_RASTERIZER_DESC desc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());

		pd3dDevice->CreateRasterizerState(&desc,&mRasterizerStateDefault);

		desc.FillMode = D3D11_FILL_WIREFRAME;

		pd3dDevice->CreateRasterizerState(&desc,&mRasterizerStateWireframe);

	}

	//Create Structured Buffer to hold the lights for the compute shader
	{
		mLightListCSSB.mBindFlags = D3D11_BIND_SHADER_RESOURCE;
		mLightListCSSB.mCPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		mLightListCSSB.mUsage = D3D11_USAGE_DYNAMIC;
		mLightListCSSB.mElements = 1024;
		mLightListCSSB.CreateBuffer(pd3dDevice);
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
		mDSV.CreateDSV(pd3dDevice,&mDepthStencil);

		//Read only DSV for stencil culling
		desc.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
		mDSVRO.mDesc = desc;
		mDSVRO.CreateDSV(pd3dDevice,&mDepthStencil);
	}
	{
		//Create DS Shader Resource View
		CD3D11_SHADER_RESOURCE_VIEW_DESC desc(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, 0, 1, 0, 1);
		mDSSRV.mDesc = desc;
		mDSSRV.CreateSRV(pd3dDevice,&mDepthStencil);
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
		mGenerator->OnD3D11CreateDevice(pd3dDevice);
		//mFXAAShader->OnD3D11CreateDevice(pd3dDevice);
		mRecompile = FALSE;
	}

	if (mSettings.wireframe) {
		pd3dImmediateContext->RSSetState(mRasterizerStateWireframe);
	}
	else {
		pd3dImmediateContext->RSSetState(mRasterizerStateDefault);
	}
	static FLOAT savedFrameTime = 1.0f/60.0f;

	//Runtime millis should be max of 15% of the frametime
	if (mGenerator->hasGeneratables()) {
		FLOAT runTime = savedFrameTime * 0.15f;
		mGenerator->Generate(pd3dDevice, pd3dImmediateContext,runTime);
	}
	else {
		savedFrameTime = fElapsedTime;
	}


	ID3D11RenderTargetView* backBuffer = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* dsv = mDSV.mDSV;

	//Clear depth stencil target
	pd3dImmediateContext->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0 );

	//Set, then draw all objects into the GBuffer
	ID3D11RenderTargetView* rtvs[2] = {mGBuffer[0].mRTV,mGBuffer[1].mRTV};
	pd3dImmediateContext->OMSetRenderTargets(2, rtvs, dsv);
	mDrawableManager.Draw(pd3dImmediateContext);

	//Make the runtime happy
	ID3D11RenderTargetView* rtvs2[2] = {NULL,NULL};
	pd3dImmediateContext->OMSetRenderTargets(2,rtvs2,dsv);

	pd3dImmediateContext->RSSetState(mRasterizerStateDefault);

	//Set up lights
	//SetUpLights(pd3dImmediateContext);
	UINT numLights = 0;
	numLights = mLightManager->updateLightBuffer(pd3dImmediateContext,mCamera,&mLightListCSSB);

	ID3D11ShaderResourceView* GBufferSRVs[4] = {mGBuffer[0].mSRV,mGBuffer[1].mSRV,mDSSRV.mSRV,mLightListCSSB.mSRV};

	//Set read only DS and proxy output texture
	//We set this here to prevent the runtime from complaining
	//We don't actually need it for the compute shader
	pd3dImmediateContext->OMSetRenderTargets(1, &mProxyTexture.mRTV, mDSVRO.mDSV);

	//Lighting CS
	mLightingCompute->Compute(pd3dImmediateContext,GBufferSRVs,&mLightingCSFBSB,mCamera,numLights);

	ID3D11ShaderResourceView* SkyboxSRVs[2] = {mDSSRV.mSRV,mLightingCSFBSB.mSRV};

	//Final lighting shader (skybox etc.)
	mLightingShader->DrawPost(pd3dImmediateContext,SkyboxSRVs,mCamera);

	//FXAA into back buffer, no need for a depth buffer to be bound
	pd3dImmediateContext->OMSetRenderTargets(1, &backBuffer, NULL);
	mFXAAShader->DrawPost(pd3dImmediateContext,mProxyTexture.mSRV);
}

void Renderer::OnExit()
{
	mLogger->log(L"Renderer OnExit");
	//TODO: cleanup
	mDrawableManager.reset();
}