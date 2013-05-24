#include "DXUT.h"
#include "DistantDrawable.h"
#include "../ShaderManager.h"
#include "../Camera.h"
#include "../MeshManager.h"
#include "Generator.h"
#include <algorithm>
#include <functional>

#define SIGNUM(X) ((X > 0) ? 1 : ((X < 0) ? -1 : 0))
#define OVERLAP_SCALE 1.05f

typedef void (*TileCreatorFunc)(DrawableState& pState,Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY,DOUBLE pPosZ,DOUBLE pTileSize,DrawableMesh* pMesh);

typedef void (*TileUpdaterFunc)(DrawableState& pState, Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY, DOUBLE pPosZ);

typedef void (*TileDrawerFunc)(BasicDrawable& pDrawable, ID3D11DeviceContext* pd3dContext);

template<typename tileType>
class LodLevel {
	DOUBLE mTileSize;
	INT mTileDim;
	INT mHTD;
	std::vector<BasicDrawable> mTiles;
	std::vector<std::pair<FLOAT,UINT> > mSortedTiles;
	LodLevel* mHigherLevel;
	Camera* mCamera;
	Generator* mGenerator;
	DOUBLE mMaxDist;
	TileCreatorFunc mTCF;
	TileUpdaterFunc mTUF;
	TileDrawerFunc mTDF;
public:
	INT mStickyOffsetX;
	INT mStickyOffsetZ;

	LodLevel(DOUBLE pTileSize, UINT pTileDimension, DrawableMesh* pMesh, DrawableShader* pShader, Camera* pCamera, Generator* pGenerator, LodLevel* pHigherLevel, TileCreatorFunc pTCF, TileUpdaterFunc pTUF, TileDrawerFunc pTDF)
		: mTileSize(pTileSize), mTileDim(pTileDimension), mHTD(pTileDimension/2), mCamera(pCamera), mHigherLevel(pHigherLevel), mGenerator(pGenerator), mTCF(pTCF), mTUF(pTUF), mTDF(pTDF)
	{
		mStickyOffsetX = 0;
		mStickyOffsetZ = 0;

		mMaxDist = mTileSize * mTileDim / 2;

		mTiles.reserve(mTileDim * mTileDim);

		//Create and add all the tiles to the list, centre of our LOD is at 0,0,0 for simplicity
		for (INT i = -mHTD; i < mHTD; i++) {
			for (INT j = -mHTD; j < mHTD; j++) {
				BasicDrawable d = BasicDrawable(pMesh,pShader,pCamera);

				DrawableState& state = d.mState;
				FLOAT scale = (FLOAT)mTileSize*OVERLAP_SCALE;
				state.mScale = DirectX::XMFLOAT3(scale,scale,scale);

				DOUBLE posX = j * mTileSize;
				DOUBLE posY = 0;
				DOUBLE posZ = i * mTileSize;
				posX += mTileSize/2;
				posZ += mTileSize/2;
				state.setPosition(posX,posY,posZ);

				mTCF(state,mGenerator,posX,posY,posZ,mTileSize,pMesh);

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

		for (INT i = 0; i < mTileDim; i++) {
			for (INT j = 0; j < mTileDim; j++) {

				BasicDrawable& d = mTiles[i * mTileDim + j];
				DrawableState& s = d.mState;

				DOUBLE posX = s.getPosX();
				DOUBLE posY = s.getPosY();
				DOUBLE posZ = s.getPosZ();

				INT shiftx = (INT)floor((DOUBLE)(mStickyOffsetX + mTileDim-1 - j)/mTileDim);
				INT shiftz = (INT)floor((DOUBLE)(mStickyOffsetZ + mTileDim-1 - i)/mTileDim);

				DOUBLE newPosX = (shiftx * mTileDim + j - mHTD) * mTileSize + hts;
				DOUBLE newPosZ = (shiftz * mTileDim + i - mHTD) * mTileSize + hts;

				if (newPosX != posX || newPosZ != posZ) {
					s.setPosition(newPosX,posY,newPosZ);

					mTUF(s,mGenerator,newPosX,posY,newPosZ);
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

		auto distCompare = [](std::pair<FLOAT,UINT> a, std::pair<FLOAT,UINT> b) -> BOOL {
			return a.first < b.first;
		};

		std::sort(mSortedTiles.begin(),mSortedTiles.end(),distCompare);

		//Draw recursively
		for (UINT n = 0; n < mSortedTiles.size(); n++) {
			UINT index = mSortedTiles[n].second;
			BasicDrawable& d = mTiles[mSortedTiles[n].second];

			INT i = index / mTileDim;
			INT j = index % mTileDim;

			INT shiftx = (INT)floor((DOUBLE)(mStickyOffsetX + mTileDim-1 - j)/mTileDim);
			INT shiftz = (INT)floor((DOUBLE)(mStickyOffsetZ + mTileDim-1 - i)/mTileDim);

			INT offsetX = shiftx * mTileDim + j - mHTD;
			INT offsetZ = shiftz * mTileDim + i - mHTD;

			if (d.mState.mDistantTile.unique()) {
				DrawRecursiveAtPos(offsetX,offsetZ,pd3dContext);
			}
		}
	}
private:

	BOOL DrawableAtPos(INT offsetX, INT offsetZ) {
		//Get indexes i and j from offsets
		//Double modulo because, for some reason, % actually calculates the remainder! Which can be negative! What a world!

		INT j = (((offsetX + mHTD)%mTileDim)+mTileDim)%mTileDim;
		INT i = (((offsetZ + mHTD)%mTileDim)+mTileDim)%mTileDim;

		UINT index = i*mTileDim + j;
		return mTiles[index].mState.mDistantTile.unique();
	}

	//If draw success, returns true. Else, returns false
	//YOU MUST CALL DRAWABLEATPOS FIRST TO CHECK IF IT CAN BE DRAWN
	//RESULTS ARE UNDEFINED IF YOU DRAW SOMETHING THAT CANNOT BE DRAWN
	void DrawRecursiveAtPos(INT offsetX, INT offsetZ, ID3D11DeviceContext* pd3dContext) {
		INT j = (((offsetX + mHTD)%mTileDim)+mTileDim)%mTileDim;
		INT i = (((offsetZ + mHTD)%mTileDim)+mTileDim)%mTileDim;

		UINT index = i*mTileDim + j;

		BasicDrawable& d = mTiles[index];

		if (mHigherLevel == NULL) {
			mTDF(d,pd3dContext);
		}
		else {
			INT x0 = 2*offsetX;
			INT z0 = 2*offsetZ;
			INT x1 = x0+1;
			INT z1 = z0+1;

			INT hlhtd = mHigherLevel->mHTD;
			INT sox = mHigherLevel->mStickyOffsetX;
			INT soz = mHigherLevel->mStickyOffsetZ;

			INT minX = sox - hlhtd;
			INT maxX = sox + hlhtd - 1;
			INT minZ = soz - hlhtd;
			INT maxZ = soz + hlhtd - 1;

			if (x0 < minX || x1 > maxX || z0 < minZ || z1 > maxZ) {
				mTDF(d,pd3dContext);
				return;
			}

			BOOL bl = mHigherLevel->DrawableAtPos(x0,z0);
			BOOL br = mHigherLevel->DrawableAtPos(x1,z0);
			BOOL tl = mHigherLevel->DrawableAtPos(x0,z1);
			BOOL tr = mHigherLevel->DrawableAtPos(x1,z1);

			if (bl && br && tl && tr) {
				mHigherLevel->DrawRecursiveAtPos(x0,z0,pd3dContext);
				mHigherLevel->DrawRecursiveAtPos(x1,z0,pd3dContext);
				mHigherLevel->DrawRecursiveAtPos(x0,z1,pd3dContext);
				mHigherLevel->DrawRecursiveAtPos(x1,z1,pd3dContext);
			}
			else {
				mTDF(d,pd3dContext);
			}
		}
	}
};

DistantDrawable::DistantDrawable( Camera* pCamera, ShaderManager* pShaderManager, MeshManager* pMeshManager, Generator* pGenerator, UINT pTileDimensionLength, UINT pNumLods, DOUBLE pMinTileSize) : Drawable(pCamera)
,mCityDrawable(NULL,NULL,NULL){
	DrawableShader* shader = pShaderManager->getDrawableShader("DistantGBufferShader");
	DrawableMesh* mesh = pMeshManager->getDrawableMesh("Plane16");

	mTileDimensionLength = pTileDimensionLength;
	mNumLods = pNumLods;
	mMinTileSize = pMinTileSize;

	mLods.reserve(mNumLods);

	//Fuck yeah lambdas, best goddamn things ever
	auto TCF = [](DrawableState& pState,Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY,DOUBLE pPosZ,DOUBLE pTileSize,DrawableMesh* pMesh) -> void {
		pState.mDistantTile = std::shared_ptr<DistantTile>(new DistantTile(pPosX,pPosY,pPosZ,pTileSize));
		pGenerator->InitialiseTileHighPriority(pState.mDistantTile);
	};

	auto TUF = [](DrawableState& pState, Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY, DOUBLE pPosZ) -> void {
		auto& dt = pState.mDistantTile;
		dt->mPosX = pPosX;
		dt->mPosY = pPosY;
		dt->mPosZ = pPosZ;

		//If unique, add to the generator. Else, it's already in there pending!
		if (dt.unique()) {
			pGenerator->InitialiseTile(dt);
		}
	};

	auto TDF = [](BasicDrawable& pDrawable, ID3D11DeviceContext* pd3dContext) -> void {
		pDrawable.Draw(pd3dContext);
	};

	DOUBLE tileSize = pMinTileSize;
	LodLevel<DistantTile>* prevLod = NULL;
	for (UINT i = 0; i < mNumLods; i++) {
		LodLevel<DistantTile>* ll = new LodLevel<DistantTile>(tileSize,mTileDimensionLength,mesh,shader,pCamera,pGenerator,prevLod,
			TCF,TUF,TDF);

		mLods.push_back(ll);

		prevLod = ll;
		tileSize*=2;
	}

	DrawableShader* cityShader = pShaderManager->getDrawableShader("GBufferShader");
	DrawableMesh* cityMesh = pMeshManager->getDrawableMesh("CubeMesh");

	mCityDrawable = BasicDrawable(cityMesh,cityShader,pCamera);
	
	DrawableState& state = mCityDrawable.mState;

	state.mDiffuseColour = DirectX::XMFLOAT3(1,0,0);
	state.mCityTile = std::make_shared<CityTile>(0,0,0,4096,cityMesh);
	pGenerator->InitialiseTile(state.mCityTile);
}

DistantDrawable::~DistantDrawable()
{
	for (int i = 0; i < mLods.size(); i++ ) {
		delete mLods[i];
	}
}

void DistantDrawable::Draw( ID3D11DeviceContext* pd3dContext )
{
	size_t num_lods = mLods.size();

	for (int i = 0; i < num_lods; i++) {
		mLods[i]->Update();
	}

	//Draw from the bottom up
	mLods[num_lods-1]->Draw(pd3dContext);

	if (mCityDrawable.mState.mCityTile.unique()) {
		mCityDrawable.DrawInstancedIndirect(pd3dContext);
	}
}
