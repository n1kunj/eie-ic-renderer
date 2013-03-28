#pragma once
#ifndef RENDERER_H
#define RENDERER_H
#include "DrawableManager.h"

class MessageLogger;
class DrawableMesh;
class DrawableShader;
class DeferredTileComputeShader;
class CubeMeshLoader;
class BasicDrawable;
class ShaderManager;
class Camera;

class Renderer {
public:
	Renderer(MessageLogger* mLogger);
	~Renderer();
	void init();

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );
	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
		void* pUserContext );
	void OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext );
	void OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );

	void OnD3D11DestroyDevice();
	void OnExit();

	DXGI_SURFACE_DESC mSurfaceDescription;
	DrawableManager mDrawableManager;
	MessageLogger* mLogger;
	DrawableMesh* mCubeMesh;
	CubeMeshLoader* mCubeLoader;
	ShaderManager* mShaderManager;
	Camera* mCamera;
#ifdef DEBUG
	boolean mRecompile;
#endif // DEBUG

};

#endif