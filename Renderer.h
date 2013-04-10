#pragma once
#ifndef RENDERER_H
#define RENDERER_H
#include "DrawableManager.h"
#include "Texture2D.h"
#include "Shaders/LightingCompute.h"
#include "LightManager.h"

class MessageLogger;
class DrawableMesh;
class DrawableShader;
class DeferredTileComputeShader;
class CubeMeshLoader;
class BasicDrawable;
class ShaderManager;
class Camera;
class FXAAShader;
class LightingShader;

class Renderer {
public:
	Renderer(MessageLogger* mLogger);
	~Renderer();
	void init();

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );
	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );
	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
		void* pUserContext );
	void OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext );
	void OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );

	void OnD3D11DestroyDevice();
	void OnExit();

	void SetUpLights(ID3D11DeviceContext* pd3dImmediateContext);

	DXGI_SURFACE_DESC mSurfaceDescription;
	DrawableManager mDrawableManager;
	MessageLogger* mLogger;
	DrawableMesh* mCubeMesh;
	CubeMeshLoader* mCubeLoader;
	ShaderManager* mShaderManager;
	Camera* mCamera;
	FXAAShader* mFXAAShader;
	LightingShader* mLightingShader;
	LightingCompute* mLightingCompute;

	Texture2D mProxyTexture;
	Texture2D mGBuffer[2];
	Depth2D mDepthStencil;
	Depth2DDSV mDSV;
	Depth2DDSV mDSVRO;
	Depth2DSRV mDSSRV;
	StructuredBuffer<LightingCSFB> mLightingCSFBSB;
	StructuredBuffer<PointLightGPU> mLightListCSSB;
	ID3D11DepthStencilState* mDSStateDefault;
	ID3D11DepthStencilState* mDSStateStencilWrite;
	ID3D11DepthStencilState* mDSStateStencilCull;

	PointLightGPU mLightList[1024];

	boolean mRecompile;
};

#endif