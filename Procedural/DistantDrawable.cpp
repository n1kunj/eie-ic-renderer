#include "DXUT.h"
#include "DistantDrawable.h"
#include "../ShaderManager.h"
#include "../Camera.h"
#include "../MeshManager.h"
#include "Generator.h"
#include <algorithm>

#define NUM_LODS 2
#define MIN_TILE_SIZE 256

//MUST BE EVEN ELSE UNDEFINED RESULTS!
#define TILES_PER_LOD_DIMENSION 2

#define SIGNUM(X) ((X > 0) ? 1 : ((X < 0) ? -1 : 0))

class LodLevel {
	DOUBLE mTileSize;
	UINT mTileDimension;
	std::vector<BasicDrawable> mTiles;
	std::vector<std::pair<FLOAT,UINT> > mSortedTiles;
	LodLevel* mHigherLevel;
	Camera* mCamera;
	Generator* mGenerator;
	DOUBLE mMaxDist;
public:
	INT mStickyOffsetX;
	INT mStickyOffsetZ;

	LodLevel(DOUBLE pTileSize, UINT pTileDimension, DrawableMesh* pMesh, DrawableShader* pShader, Camera* pCamera, Generator* pGenerator, LodLevel* pHigherLevel)
		: mTileSize(pTileSize), mTileDimension(pTileDimension), mCamera(pCamera), mHigherLevel(pHigherLevel), mGenerator(pGenerator)
	{
		mStickyOffsetX = 0;
		mStickyOffsetZ = 0;

		mMaxDist = mTileSize * mTileDimension / 2;

		mTiles.reserve(mTileDimension * mTileDimension);

		INT halfTD = mTileDimension/2;
		//Create and add all the tiles to the list, centre of our LOD is at 0,0,0 for simplicity
		for (INT i = -halfTD; i < halfTD; i++) {
			for (INT j = -halfTD; j < halfTD; j++) {
				BasicDrawable d = BasicDrawable(pMesh,pShader,pCamera);

				DrawableState& state = d.mState;

				state.mScale = DirectX::XMFLOAT3((FLOAT)mTileSize, (FLOAT)mTileSize, (FLOAT)mTileSize);

				DOUBLE posX = j * mTileSize;
				DOUBLE posY = 0;
				DOUBLE posZ = i * mTileSize;
				posX += mTileSize/2;
				posZ += mTileSize/2;
				state.setPosition(posX,posY,posZ);

				state.mDistantTextures = std::shared_ptr<DistantTextures>(new DistantTextures(posX,posY,posZ,mTileSize));

				pGenerator->InitialiseDistantTile(state.mDistantTextures);

				mTiles.push_back(d);
			}
		}

		//Update all tiles to where they need to be
		Update();

		for (int i = 0; i < mTiles.size(); i++) {
			mSortedTiles.push_back(std::pair<FLOAT,UINT>(0.0f,i));
		}
	}

	~LodLevel() {
	}

	void Update() {
		INT oldX = mStickyOffsetX;
		INT oldZ = mStickyOffsetZ;

		DOUBLE camX = mCamera->getEyeX();
		DOUBLE camZ = mCamera->getEyeZ();

		DOUBLE hts = mTileSize/2;

		mStickyOffsetX = (INT)((camX+SIGNUM(camX)*hts)/mTileSize);
		mStickyOffsetZ = (INT)((camZ+SIGNUM(camZ)*hts)/mTileSize);

		//Early return if we haven't moved
		if (mStickyOffsetX == oldX && mStickyOffsetZ == oldZ) {
			return;
		}

		for (INT i = 0; i < mTileDimension; i++) {
			for (INT j = 0; j < mTileDimension; j++) {

				BasicDrawable& d = mTiles[i * mTileDimension + j];
				DrawableState& s = d.mState;

				DOUBLE posX = s.getPosX();
				DOUBLE posY = s.getPosY();
				DOUBLE posZ = s.getPosZ();

				INT shiftx = (INT)((mStickyOffsetX + (mTileDimension-1) - j)/mTileDimension);
				INT shiftz = (INT)((mStickyOffsetZ + (mTileDimension-1) - i)/mTileDimension);

				INT td = mTileDimension;
				INT htd = mTileDimension/2;

				DOUBLE newPosX = (shiftx * td + j - htd) * mTileSize + hts;

				DOUBLE newPosZ = (shiftz * td + i - htd) * mTileSize + hts;

				if (newPosX != posX || newPosZ != posZ) {
					s.setPosition(newPosX,posY,newPosZ);

					auto& dt = s.mDistantTextures;
					dt->mPosX = newPosX;
					dt->mPosZ = newPosZ;

					//If unique, add to the generator. Else, it's already in there pending!
					if (dt.unique()) {
						mGenerator->InitialiseDistantTile(dt);
					}
				}
			}
		}
	}

	void Draw(ID3D11DeviceContext* pd3dContext) {

		//Sort by distance to avoid overdraw
		DOUBLE camX = mCamera->getEyeX();
		DOUBLE camZ = mCamera->getEyeZ();

		for (int n = 0; n < mSortedTiles.size(); n++) {
			DrawableState &s = mTiles[mSortedTiles[n].second].mState;

			DOUBLE distX = s.getPosX() - camX;
			DOUBLE distZ = s.getPosZ() - camZ;
			FLOAT distsq = (FLOAT)(distX * distX + distZ * distZ);

			mSortedTiles[n].first = distsq;
		}

		std::sort(mSortedTiles.begin(),mSortedTiles.end(),distCompare);

		DOUBLE halfTileSize = mTileSize/2;

		//Draw recursively
		for (UINT n = 0; n < mSortedTiles.size(); n++) {
			UINT index = mSortedTiles[n].second;
			BasicDrawable& d = mTiles[mSortedTiles[n].second];

			INT i = index / mTileDimension;
			INT j = index % mTileDimension;
			INT shiftx = (INT)((mStickyOffsetX + (mTileDimension-1) - j)/mTileDimension);
			INT shiftz = (INT)((mStickyOffsetZ + (mTileDimension-1) - i)/mTileDimension);

			INT td = mTileDimension;
			INT htd = mTileDimension/2;

			INT offsetX = shiftx * td + j - htd;
			INT offsetZ = shiftz * td + i - htd;

			//If the texture hasn't been created yet, don't draw anything
			if (!d.mState.mDistantTextures.unique()) {
				continue;
			}
			if (mHigherLevel == NULL) {
				d.Draw(pd3dContext);
			}
			else {
				DOUBLE x1 = d.mState.getPosX();
				DOUBLE z1 = d.mState.getPosZ();
				DOUBLE x0 = x1 - halfTileSize;
				DOUBLE z0 = z1 - halfTileSize;

				BOOL bl = mHigherLevel->DrawableAtPos(x0,z0);
				BOOL br = mHigherLevel->DrawableAtPos(x1,z0);
				BOOL tl = mHigherLevel->DrawableAtPos(x0,z1);
				BOOL tr = mHigherLevel->DrawableAtPos(x1,z1);

				if (bl && br && tl && tr) {
					DrawRecursiveAtPos(x0,z0,pd3dContext);
					DrawRecursiveAtPos(x1,z0,pd3dContext);
					DrawRecursiveAtPos(x0,z1,pd3dContext);
					DrawRecursiveAtPos(x1,z1,pd3dContext);
				}
				else {
					d.Draw(pd3dContext);
				}
			}
		}
	}
private:

	BOOL DrawableAtPos(DOUBLE x, DOUBLE z) {
		//First bounds check
		//if ( abs(x - mStickyCamX) > mMaxDist || abs(z - mStickyCamZ) > mMaxDist) {
		//	return FALSE;
		//}

		//First find the drawable with a bottom left corner at x and z

		return FALSE;
	}

	//If draw success, returns true. Else, returns false
	//YOU MUST CALL DRAWABLEATPOS FIRST TO CHECK IF IT CAN BE DRAWN
	//RESULTS ARE UNDEFINED IF YOU DRAW SOMETHING THAT CANNOT BE DRAWN
	void DrawRecursiveAtPos(DOUBLE x, DOUBLE z, ID3D11DeviceContext* pd3dContext) {

	}

	static BOOL distCompare(std::pair<FLOAT,UINT> a, std::pair<FLOAT,UINT> b)
	{
		return a.first < b.first;
	}
};

DistantDrawable::DistantDrawable( Camera* pCamera, ShaderManager* pShaderManager, MeshManager* pMeshManager, Generator* pGenerator, UINT pTileDimensionLength, DOUBLE pTileSize, DOUBLE pMinDrawDistance, DOUBLE pMaxDrawDistance) : Drawable(pCamera)
{
	//mGenerator = pGenerator;
	DrawableShader* shader = pShaderManager->getDrawableShader("DistantGBufferShader");
	DrawableMesh* mesh = pMeshManager->getDrawableMesh("Plane16");
	DOUBLE tileSize = MIN_TILE_SIZE;
	LodLevel* prevLod = NULL;
	mLods.reserve(NUM_LODS);
	for (int i = 0; i < NUM_LODS; i++) {
		LodLevel* ll = new LodLevel(tileSize,TILES_PER_LOD_DIMENSION,mesh,shader,pCamera,pGenerator,prevLod);

		mLods.push_back(ll);

		prevLod = ll;
		tileSize*=2;
	}

}

DistantDrawable::~DistantDrawable()
{
	for (int i = 0; i < mLods.size(); i++ ) {
		delete mLods[i];
	}
}

void DistantDrawable::Draw( ID3D11DeviceContext* pd3dContext )
{
	for (int i = 0; i < NUM_LODS; i++) {
		mLods[i]->Update();
	}

	//Draw from the bottom up
	LodLevel& ll = *mLods[NUM_LODS-1];

	ll.Draw(pd3dContext);
}
