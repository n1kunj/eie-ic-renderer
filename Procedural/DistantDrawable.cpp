#include "DXUT.h"
#include "DistantDrawable.h"
#include "../ShaderManager.h"
#include "../Camera.h"
#include "../MeshManager.h"
#include "Generator.h"

#define TILE_DIMENSION_LENGTH 21
#define NUM_TILES TILE_DIMENSION_LENGTH*TILE_DIMENSION_LENGTH
#define TILE_SIZE 512.0
#define MESH_SCALE TILE_SIZE

DistantDrawable::DistantDrawable( Camera* pCamera, ShaderManager* pShaderManager, MeshManager* pMeshManager, Generator* pGenerator ) : Drawable(pCamera)
{
	mShader = pShaderManager->getDrawableShader("DistantGBufferShader");
	mDrawables.reserve(NUM_TILES);

	DrawableMesh* mesh = pMeshManager->getDrawableMesh("Plane16");

	DOUBLE posx = mCamera->getEyeX();
	DOUBLE posy = 0;
	DOUBLE posz = mCamera->getEyeZ();

	INT tilex = -TILE_DIMENSION_LENGTH/2;
	INT tilez = -TILE_DIMENSION_LENGTH/2;

	for (int i = 0; i < NUM_TILES; i++) {
		mDrawables.push_back(BasicDrawable(mesh,mShader,pCamera));
		mDrawables[i].mState.setPosition(posx + TILE_SIZE * tilex, posy, posz + TILE_SIZE * tilez);
		mDrawables[i].mState.mScale = DirectX::XMFLOAT3(MESH_SCALE,1.0f,MESH_SCALE);
		mDrawables[i].mState.mDistantTextures = std::shared_ptr<DistantTextures>(new DistantTextures());

		pGenerator->InitialiseDistantTile(mDrawables[i].mState.mDistantTextures);

		tilex++;
		if (tilex > TILE_DIMENSION_LENGTH/2) {
			tilez++;
			tilex = -TILE_DIMENSION_LENGTH/2;
		}
	}
}

DistantDrawable::~DistantDrawable()
{
}

void DistantDrawable::Draw( ID3D11DeviceContext* pd3dContext )
{
	for (int i = 0; i < mDrawables.size(); i++) {
		mDrawables[i].Draw(pd3dContext);
	}
	//for (int i = 0; i < 500; i++) {
		//mDrawables[0].Draw(pd3dContext);
	//}
}
