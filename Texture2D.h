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


#endif