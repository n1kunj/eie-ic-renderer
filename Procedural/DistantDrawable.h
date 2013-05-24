#include "DXUT.h"
#pragma once
#ifndef DISTANT_DRAWABLE_H
#define DISTANT_DRAWABLE_H
#include "..\Drawable.h"
#include <vector>

class ShaderManager;
class MeshManager;
class Generator;
template<typename tileType>
class LodLevel;

class DistantDrawable : public Drawable {
public:
	DistantDrawable::DistantDrawable( Camera* pCamera, ShaderManager* pShaderManager, MeshManager* pMeshManager, Generator* pGenerator, UINT pTileDimensionLength, UINT pNumLods, DOUBLE pMinTileSize);
	void Draw(ID3D11DeviceContext* pd3dContext);
	virtual ~DistantDrawable();
private:
	DistantDrawable(const DistantDrawable& copy);
	std::vector<LodLevel<DistantTile>*> mLods;
	LodLevel<CityTile>* mCityLods;
	//MUST BE EVEN ELSE UNDEFINED RESULTS
	UINT mTileDimensionLength;
	UINT mNumLods;
	DOUBLE mMinTileSize;
};


#endif