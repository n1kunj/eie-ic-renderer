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

template <typename T>
class StructuredBuffer {
public:
	UINT mBindFlags;
	D3D11_USAGE mUsage;
	UINT mElements;
	UINT mCPUAccessFlags;
	ID3D11Buffer* mBuffer;
	ID3D11ShaderResourceView* mSRV;
	ID3D11UnorderedAccessView* mUAV;

	StructuredBuffer() : mBuffer(NULL), mSRV(NULL), mUAV(NULL) {}

	void CreateBuffer(ID3D11Device* pd3dDevice) {
		OnD3D11DestroyDevice();
		D3D11_BUFFER_DESC desc;
		::ZeroMemory (&desc, sizeof (desc));
		desc.BindFlags = mBindFlags;
		desc.Usage = mUsage;
		desc.ByteWidth = mElements * sizeof(T);
		desc.CPUAccessFlags = mCPUAccessFlags;
		desc.StructureByteStride = sizeof(T);
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		pd3dDevice->CreateBuffer(&desc,0,&mBuffer);
		if (mBindFlags & D3D11_BIND_SHADER_RESOURCE) {
			pd3dDevice->CreateShaderResourceView(mBuffer,0,&mSRV);
		}
		if (mBindFlags & D3D11_BIND_UNORDERED_ACCESS) {
			pd3dDevice->CreateUnorderedAccessView(mBuffer,0,&mUAV);
		}
	}

	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mBuffer);
		SAFE_RELEASE(mSRV);
		SAFE_RELEASE(mUAV);
	}

	T* MapDiscard(ID3D11DeviceContext* pd3dContext)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		pd3dContext->Map(mBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		return static_cast<T*>(mappedResource.pData);
	}

	void Unmap(ID3D11DeviceContext* pd3dContext)
	{
		pd3dContext->Unmap(mBuffer, 0);
	}

	~StructuredBuffer() {
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

	void CreateDSV(ID3D11Device* pd3dDevice, Depth2D* pDepthStencil) {
		OnD3D11DestroyDevice();
		pd3dDevice->CreateDepthStencilView(pDepthStencil->mTexture, &mDesc, &mDSV);
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

	void CreateSRV(ID3D11Device* pd3dDevice, Depth2D* pDepthStencil) {
		OnD3D11DestroyDevice();
		pd3dDevice->CreateShaderResourceView(pDepthStencil->mTexture, &mDesc, &mSRV);
	}

	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mSRV);
	}

	~Depth2DSRV() {
		OnD3D11DestroyDevice();
	}
};


#endif