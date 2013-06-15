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

class DistantTile {
public:
	DOUBLE mPosX;
	DOUBLE mPosY;
	DOUBLE mPosZ;
	DOUBLE mSize;

	Texture2D mAlbedoMap;
	Texture2D mNormalMap;
	Texture2D mHeightMap;

	DistantTile(DOUBLE pPosX, DOUBLE pPosY, DOUBLE pPosZ, DOUBLE pSize);
	~DistantTile() {}
};

enum CityLodLevel {
	CITY_LOD_LEVEL_HIGH = 0,
	CITY_LOD_LEVEL_MED = 1,
	CITY_LOD_LEVEL_LOW = 2,
	CITY_LOD_LEVEL_XLOW = 3,
	CITY_LOD_LEVEL_XXLOW = 4
};
const UINT MAX_BUILDINGS_PER_TILE[] = {16,1,1,1,1};

class CityTile {
public:
	DOUBLE mPosX;
	DOUBLE mPosY;
	DOUBLE mPosZ;
	DOUBLE mSize;
	CityLodLevel mCLL;

	StructuredBuffer<InstanceData> mInstanceBuffer;
	StructuredBuffer<UINT> mIndirectBuffer;
	UINT mIndirectData[5];
	//Position is the centre of the tile
	CityTile(DOUBLE pPosX, DOUBLE pPosY, DOUBLE pPosZ, DOUBLE pSize, DrawableMesh* pMesh, CityLodLevel pCLL);
	~CityTile() {}
};

typedef std::shared_ptr<DistantTile> DTPTR;
typedef std::shared_ptr<CityTile> CTPTR;
class Generator {
private:
	std::deque<DTPTR> mTextureQueue;
	std::deque<DTPTR> mTextureQueueHP;
	ID3D11Query* dtDisjoint;
	ID3D11Query* dtStart;
	ID3D11Query* dtEnd;

	std::deque<CTPTR> mCityQueue;
	std::deque<CTPTR> mCityQueueHP;
	ID3D11Query* ctDisjoint;
	ID3D11Query* ctStart;
	ID3D11Query* ctEnd;

	boolean mCompiled;
	ID3D11ComputeShader* mCSDistant;
	ID3D11ComputeShader* mCSCity;
	ID3D11Buffer* mCSCBDistant;
	ID3D11Buffer* mCSCBCity;
	MessageLogger* mLogger;
	UINT mSimplex2DLUT[256][256];
	StructuredBuffer<UINT> mSimplexBuffer;
	BOOL mSimplexInit;
	BOOL mInitialLoad;
	UINT64 mFrameNumber;
public:
	HRESULT OnD3D11CreateDevice( ID3D11Device* pd3dDevice );
	void OnD3D11DestroyDevice() {
		SAFE_RELEASE(mCSDistant);
		SAFE_RELEASE(mCSCity);
		SAFE_RELEASE(mCSCBDistant);
		SAFE_RELEASE(mCSCBCity);

		SAFE_RELEASE(dtDisjoint);
		SAFE_RELEASE(dtStart);
		SAFE_RELEASE(dtEnd);

		SAFE_RELEASE(ctDisjoint);
		SAFE_RELEASE(ctStart);
		SAFE_RELEASE(ctEnd);

		mCompiled = FALSE;
		mSimplexInit = FALSE;
		mSimplexBuffer.OnD3D11DestroyDevice();
		mTextureQueue.clear();
		mTextureQueueHP.clear();
		mCityQueue.clear();
		mCityQueueHP.clear();
	}

	Generator(MessageLogger* pLogger);
	~Generator() {
		OnD3D11DestroyDevice();
	}

	void InitialiseTile(DTPTR const& pDistantTile) {
		mTextureQueue.push_back(pDistantTile);
	}
	void InitialiseTileHighPriority(DTPTR const& pDistantTile) {
		mTextureQueueHP.push_back(pDistantTile);
	}
	void InitialiseTile(CTPTR const& pCityTile) {
		mCityQueue.push_back(pCityTile);
	}
	void InitialiseTileHighPriority(CTPTR const& pCityTile) {
		mCityQueueHP.push_back(pCityTile);
	}
	void setInitialLoad() {
		mInitialLoad = TRUE;
	}

	BOOL hasGeneratables() {
		size_t sum = 0;
		sum+=mTextureQueue.size();
		sum+=mTextureQueueHP.size();
		sum+=mCityQueue.size();
		sum+=mCityQueueHP.size();
		return (BOOL)sum;
	}

	void Generate(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, FLOAT pMaxRuntimeSeconds);

	UINT GetMinCityTileDim();

private:
	FLOAT ProcessDT(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, std::deque<DTPTR> &pDTqueue);
	FLOAT ProcessCT(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, std::deque<CTPTR> &pCTqueue);
	void ComputeTextures(ID3D11DeviceContext* pd3dContext, DistantTile &pDT );

	void InitialiseSimplex( ID3D11DeviceContext* pd3dContext );
	void ComputeCity(ID3D11DeviceContext* pd3dContext, CityTile &pCT);
};

#endif