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
	ID3D11UnorderedAccessView* mUAV;

	Texture2D() : mTexture(NULL), mRTV(NULL), mSRV(NULL), mUAV(NULL) {}

	void CreateTexture(ID3D11Device* pd3dDevice) {
		CreateTexture(pd3dDevice,NULL);
	}

	void CreateTexture(ID3D11Device* pd3dDevice, const D3D11_SUBRESOURCE_DATA* pSubResourceData) {
		OnD3D11DestroyDevice();
		pd3dDevice->CreateTexture2D(&mDesc,pSubResourceData,&mTexture);
		if (mDesc.BindFlags & D3D11_BIND_RENDER_TARGET) {
			pd3dDevice->CreateRenderTargetView(mTexture, 0, &mRTV);
		}
		if (mDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) {
			pd3dDevice->CreateShaderResourceView(mTexture, 0, &mSRV);
		}
		if (mDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) {
			pd3dDevice->CreateUnorderedAccessView(mTexture, 0, &mUAV);
		}
	}

	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mTexture);
		SAFE_RELEASE(mSRV);
		SAFE_RELEASE(mRTV);
		SAFE_RELEASE(mUAV);
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
	UINT mMiscFlags;
	ID3D11Buffer* mBuffer;
	BOOL mDefaultSRVDesc;
	BOOL mDefaultUAVDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC mSRVDesc;
	D3D11_UNORDERED_ACCESS_VIEW_DESC mUAVDesc;
	ID3D11ShaderResourceView* mSRV;
	ID3D11UnorderedAccessView* mUAV;

	StructuredBuffer() : mBuffer(NULL), mSRV(NULL), mUAV(NULL),mBindFlags(0),mElements(0),mUsage(D3D11_USAGE_DEFAULT),mCPUAccessFlags(0),mMiscFlags(0),
	mDefaultSRVDesc(TRUE),mDefaultUAVDesc(TRUE){
		::ZeroMemory (&mSRVDesc, sizeof (mSRVDesc));
		::ZeroMemory (&mUAVDesc, sizeof (mUAVDesc));
	}


	void CreateBuffer(ID3D11Device* pd3dDevice) {
		CreateBuffer(pd3dDevice,NULL);
	}

	void CreateBuffer(ID3D11Device* pd3dDevice, VOID* pInitData) {
		OnD3D11DestroyDevice();
		D3D11_BUFFER_DESC desc;
		::ZeroMemory (&desc, sizeof (desc));
		desc.BindFlags = mBindFlags;
		desc.Usage = mUsage;
		desc.ByteWidth = mElements * sizeof(T);
		desc.CPUAccessFlags = mCPUAccessFlags;
		desc.StructureByteStride = sizeof(T);
		desc.MiscFlags = mMiscFlags;

		if (!(mMiscFlags & D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS) && !(mBindFlags & D3D11_BIND_VERTEX_BUFFER)) {
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		}

		if (pInitData!=NULL) {
			D3D11_SUBRESOURCE_DATA initData;
			ZeroMemory( &initData, sizeof(initData) );
			initData.pSysMem = pInitData;

			pd3dDevice->CreateBuffer(&desc,&initData,&mBuffer);
		}
		else {
			pd3dDevice->CreateBuffer(&desc,0,&mBuffer);
		}

		if (mBindFlags & D3D11_BIND_SHADER_RESOURCE) {
			D3D11_SHADER_RESOURCE_VIEW_DESC* srvd = mDefaultSRVDesc ? NULL : &mSRVDesc;
			pd3dDevice->CreateShaderResourceView(mBuffer,srvd,&mSRV);
		}
		if (mBindFlags & D3D11_BIND_UNORDERED_ACCESS) {
			D3D11_UNORDERED_ACCESS_VIEW_DESC* uavd = mDefaultUAVDesc ? NULL : &mUAVDesc;
			pd3dDevice->CreateUnorderedAccessView(mBuffer,uavd,&mUAV);
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