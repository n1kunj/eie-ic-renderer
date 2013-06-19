#pragma once
#ifndef RENDERER_H
#define RENDERER_H
#include "DrawableManager.h"
#include "Texture2D.h"
#include "LightManager.h"
#include "Shaders/LightingCompute.h"

class MessageLogger;
class ShaderManager;
class MeshManager;
class Camera;
class FXAAShader;
class LightingShader;
class Generator;

struct RendererSettings {
	BOOL wireframe;
	DOUBLE cameraspeed;
	FLOAT znear;
	FLOAT zfar;

	RendererSettings() {
		wireframe = FALSE;
		cameraspeed = 1000;
		zfar = 24000000.0;
		znear = 8.0;
	}
};

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

	RendererSettings mSettings;

	DXGI_SURFACE_DESC mSurfaceDescription;
	DrawableManager mDrawableManager;
	MessageLogger* mLogger;
	ShaderManager* mShaderManager;
	LightManager* mLightManager;
	MeshManager* mMeshManager;
	Camera* mCamera;
	FXAAShader* mFXAAShader;
	LightingShader* mLightingShader;
	LightingCompute* mLightingCompute;
	Generator* mGenerator;

	Texture2D mProxyTexture;
	Texture2D mGBuffer[3];
	Depth2D mDepthStencil;
	Depth2DDSV mDSV;
	Depth2DDSV mDSVRO;
	Depth2DSRV mDSSRV;
	StructuredBuffer<LightingCSFB> mLightingCSFBSB;
	StructuredBuffer<PointLightGPU> mLightListCSSB;
	ID3D11DepthStencilState* mDSStateDefault;
	ID3D11DepthStencilState* mDSStateStencilWrite;
	ID3D11DepthStencilState* mDSStateStencilCull;
	ID3D11RasterizerState* mRasterizerStateDefault;
	ID3D11RasterizerState* mRasterizerStateWireframe;

	PointLight mLightList[1024];

	boolean mRecompile;
private:
	Renderer(const Renderer& copy);
};

#endif