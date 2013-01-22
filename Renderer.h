#pragma once
#ifndef RENDERER_H
#define RENDERER_H

class RendererImplementation;
class MessageLogger;

class Renderer {
public:
	Renderer(MessageLogger* logger);
	~Renderer();

	void init();
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice);
	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );
	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
		void* pUserContext );
	void OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
	void OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext );
	void OnD3D11DestroyDevice();
	void OnExit();
private:
	RendererImplementation* _impl;
};

#endif