#pragma once
#ifndef TEXTURE2D_H
#define TEXTURE2D_H
#include "DXUT.h"

class Texture2D {
public:
	D3D11_TEXTURE2D_DESC mDesc;
	ID3D11Texture2D* mTexture;
	ID3D11ShaderResourceView* mSRV;
	ID3D11RenderTargetView* mRTV;

	Texture2D() : mTexture(NULL), mRTV(NULL), mSRV(NULL) {}

	void CreateTexture(ID3D11Device* pd3dDevice) {
		OnD3D11DestroyDevice();
		pd3dDevice->CreateTexture2D(&mDesc,0,&mTexture);
		pd3dDevice->CreateRenderTargetView(mTexture, 0, &mRTV);
		pd3dDevice->CreateShaderResourceView(mTexture, 0, &mSRV);
	}

	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mTexture);
		SAFE_RELEASE(mSRV);
		SAFE_RELEASE(mRTV);
	}

	~Texture2D() {
		OnD3D11DestroyDevice();
	}
};

class Depth2D {
public:
	D3D11_TEXTURE2D_DESC mDesc;
	ID3D11Texture2D* mTexture;

	Depth2D() : mTexture(NULL){}

	void CreateTexture(ID3D11Device* pd3dDevice) {
		OnD3D11DestroyDevice();
		pd3dDevice->CreateTexture2D(&mDesc,0,&mTexture);
	}

	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mTexture);
	}

	~Depth2D() {
		OnD3D11DestroyDevice();
	}
};

class Depth2DDSV {
public:
	D3D11_DEPTH_STENCIL_VIEW_DESC mDesc;
	ID3D11DepthStencilView* mDSV;

	Depth2DDSV() : mDSV(NULL){}

	void CreateDSV(ID3D11Device* pd3dDevice, Depth2D pDepthStencil) {
		OnD3D11DestroyDevice();
		pd3dDevice->CreateDepthStencilView(pDepthStencil.mTexture, &mDesc, &mDSV);
	}

	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mDSV);
	}

	~Depth2DDSV() {
		OnD3D11DestroyDevice();
	}
};

class Depth2DSRV {
public:
	D3D11_SHADER_RESOURCE_VIEW_DESC mDesc;
	ID3D11ShaderResourceView* mSRV;

	Depth2DSRV() : mSRV(NULL){}

	void CreateSRV(ID3D11Device* pd3dDevice, Depth2D pDepthStencil) {
		OnD3D11DestroyDevice();
		pd3dDevice->CreateShaderResourceView(pDepthStencil.mTexture, &mDesc, &mSRV);
	}

	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mSRV);
	}

	~Depth2DSRV() {
		OnD3D11DestroyDevice();
	}
};


#endif