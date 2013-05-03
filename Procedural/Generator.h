#include "DXUT.h"
#pragma once
#ifndef GENERATOR_H
#define GENERATOR_H
#include <deque>
#include <memory>
#include "../Texture2D.h"
#include "../DirectXMath/DirectXMath.h"
#include "../MessageProcessor.h"
#include "../DrawableMesh.h"

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

class CityTile {
public:
	DOUBLE mPosX;
	DOUBLE mPosY;
	DOUBLE mPosZ;
	DOUBLE mSize;

	StructuredBuffer<InstanceData> mInstanceBuffer;
	StructuredBuffer<UINT> mIndirectBuffer;
	UINT mIndirectData[5];
	//Position is the centre of the tile
	CityTile(DOUBLE pPosX, DOUBLE pPosY, DOUBLE pPosZ, DOUBLE pSize, DrawableMesh* pMesh);
	~CityTile() {}
};

typedef std::shared_ptr<DistantTextures> DTPTR;
typedef std::shared_ptr<CityTile> CTPTR;
class Generator {
private:
	std::deque<DTPTR> mTextureQueue;
	std::deque<CTPTR> mCityQueue;
	boolean mCompiled;
	ID3D11ComputeShader* mCSDistant;
	ID3D11ComputeShader* mCSCity;
	ID3D11Buffer* mCSCBDistant;
	ID3D11Buffer* mCSCBCity;
	MessageLogger* mLogger;
	UINT mSimplex2DLUT[256][256];
	StructuredBuffer<UINT> mSimplexBuffer;
	BOOL mSimplexInit;
public:
	HRESULT OnD3D11CreateDevice( ID3D11Device* pd3dDevice );
	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mCSDistant);
		SAFE_RELEASE(mCSCity);
		SAFE_RELEASE(mCSCBDistant);
		SAFE_RELEASE(mCSCBCity);
		mCompiled = FALSE;
		mSimplexInit = FALSE;
		mSimplexBuffer.OnD3D11DestroyDevice();
	}

	Generator(MessageLogger* pLogger);
	~Generator() {
		OnD3D11DestroyDevice();
		mTextureQueue.clear();
	}

	void InitialiseDistantTile(DTPTR const& pDistantTexture) {
		mTextureQueue.push_back(pDistantTexture);
	}
	void InitialiseDistantTileHighPriority(DTPTR const& pDistantTexture) {
		mTextureQueue.push_front(pDistantTexture);
	}
	void InitialiseCityTile(CTPTR const& pCityTile) {
		mCityQueue.push_back(pCityTile);
	}

	void Generate(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, UINT pMaxRuntimeMillis);

private:
	void ProcessDT(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext);
	void ProcessCT(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext);
	void ComputeTextures(ID3D11DeviceContext* pd3dContext, DistantTextures &pDT );
	void ComputeCity(ID3D11DeviceContext* pd3dContext, CityTile &pCT);
};

#endif