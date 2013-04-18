#include "DXUT.h"
#pragma once
#ifndef GENERATOR_H
#define GENERATOR_H
#include <deque>
#include <memory>
#include "../Texture2D.h"
#include "../DirectXMath/DirectXMath.h"
#include "../MessageProcessor.h"

class DistantTextures {
public:
	DOUBLE mPosX;
	DOUBLE mPosY;
	DOUBLE mPosZ;
	DOUBLE mSize;

	Texture2D mAlbedoMap;
	Texture2D mNormalMap;
	Texture2D mHeightMap;

	DistantTextures(DOUBLE pPosX, DOUBLE pPosY, DOUBLE pPosZ, DOUBLE pSize);
	~DistantTextures() {}
};

typedef std::shared_ptr<DistantTextures> DTPTR;

class Generator {
private:
	std::deque<DTPTR> mTextureQueue;
	void Generator::ComputeTextures(ID3D11DeviceContext* pd3dContext, DistantTextures &pDT );
	boolean mCompiled;
	ID3D11ComputeShader* mCS;
	ID3D11Buffer* mCSCB;
	MessageLogger* mLogger;
public:
	HRESULT OnD3D11CreateDevice( ID3D11Device* pd3dDevice );
	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mCS);
		SAFE_RELEASE(mCSCB);
		mCompiled = FALSE;
	}

	Generator(MessageLogger* pLogger) : mCompiled(FALSE),mCS(NULL),mCSCB(NULL), mLogger(pLogger) {}
	~Generator() {
		OnD3D11DestroyDevice();
		mTextureQueue.clear();
	}

	void InitialiseDistantTile(DTPTR const& pDistantTexture) {
		mTextureQueue.push_back(pDistantTexture);
	}

	void Generate(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, UINT pMaxRuntimeMillis);
};

#endif