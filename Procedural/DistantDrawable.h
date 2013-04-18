#include "DXUT.h"
#pragma once
#ifndef DISTANT_DRAWABLE_H
#define DISTANT_DRAWABLE_H
#include "..\Drawable.h"
#include <vector>

class ShaderManager;
class MeshManager;
class Generator;

class DistantDrawable : public Drawable {
public:
	DistantDrawable( Camera* pCamera, ShaderManager* pShaderManager, MeshManager* pMeshManager, Generator* pGenerator, UINT pTileDimensionLength, DOUBLE pTileSize, DOUBLE pMinDrawDistance, DOUBLE pMaxDrawDistance);
	void Draw(ID3D11DeviceContext* pd3dContext);
	virtual ~DistantDrawable();
private:
	void Update();
	DistantDrawable(const DistantDrawable& copy);
	DrawableShader* mShader;
	std::vector<BasicDrawable> mDrawables;
	Generator* mGenerator;
	UINT mTileDimensionLength;
	UINT mNumTiles;
	DOUBLE mTileSize;
	DOUBLE mMeshScale;
	DOUBLE mMinDrawDistance;
	DOUBLE mMaxDrawDistance;
	DOUBLE mStickyCamX;
	DOUBLE mStickyCamY;
	DOUBLE mStickyCamZ;
};


#endif