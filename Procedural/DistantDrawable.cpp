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

typedef void (*TileDrawerFunc)(DOUBLE pTileSize, BasicDrawable& pDrawable, ID3D11DeviceContext* pd3dContext);

typedef BOOL (*TileUniqueFunc)(DrawableState& pState);

template<typename tileType>
class LodLevel {
	DOUBLE mTileSize;
	INT mTileDim;
	INT mHTD;
	FLOAT mOverlapScale;
	std::vector<BasicDrawable> mTiles;
	std::vector<std::pair<FLOAT,UINT> > mSortedTiles;
	LodLevel* mHigherLevel;
	Camera* mCamera;
	Generator* mGenerator;
	DOUBLE mMaxDist;
	TileCreatorFunc mTCF;
	TileUpdaterFunc mTUF;
	TileDrawerFunc mTDF;
	TileUniqueFunc mTUniqueF;
public:
	INT mStickyOffsetX;
	INT mStickyOffsetZ;

	LodLevel(DOUBLE pTileSize, UINT pTileDimension, FLOAT pOverlapScale, DrawableMesh* pMesh, DrawableShader* pShader, Camera* pCamera, Generator* pGenerator, LodLevel* pHigherLevel, TileCreatorFunc pTCF, TileUpdaterFunc pTUF, TileDrawerFunc pTDF, TileUniqueFunc pTUniqueF)
		: mTileSize(pTileSize), mTileDim(pTileDimension), mHTD(pTileDimension/2), mOverlapScale(pOverlapScale), mCamera(pCamera), mHigherLevel(pHigherLevel), mGenerator(pGenerator), mTCF(pTCF), mTUF(pTUF), mTDF(pTDF), mTUniqueF(pTUniqueF)
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
				FLOAT scale = (FLOAT)mTileSize*mOverlapScale;
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

			if (mTUniqueF(d.mState)) {
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
		return mTUniqueF(mTiles[index].mState);
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
			mTDF(mTileSize,d,pd3dContext);
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
				mTDF(mTileSize,d,pd3dContext);
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
				mTDF(mTileSize,d,pd3dContext);
			}
		}
	}
};

DistantDrawable::DistantDrawable( Camera* pCamera, ShaderManager* pShaderManager, MeshManager* pMeshManager, Generator* pGenerator, UINT pTileDimensionLength, UINT pNumLods, DOUBLE pMinTileSize) : Drawable(pCamera)
{
	pGenerator->setInitialLoad();

	{
		//Initialise terrain

		DrawableShader* shader = pShaderManager->getDrawableShader("DistantGBufferShader");
		DrawableMesh* mesh = pMeshManager->getDrawableMesh("Plane16");

		mTileDimensionLength = pTileDimensionLength;
		mNumLods = pNumLods;
		mMinTileSize = pMinTileSize;

		mLods.reserve(mNumLods);

		auto TCF = [](DrawableState& pState,Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY,DOUBLE pPosZ,DOUBLE pTileSize,DrawableMesh* pMesh) -> void {
			pState.mDistantTile = std::shared_ptr<DistantTile>(new DistantTile(pPosX,pPosY,pPosZ,pTileSize));
			pGenerator->InitialiseTile(pState.mDistantTile);
		};

		auto TCFHP = [](DrawableState& pState,Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY,DOUBLE pPosZ,DOUBLE pTileSize,DrawableMesh* pMesh) -> void {
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

		auto TUFHP = [](DrawableState& pState, Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY, DOUBLE pPosZ) -> void {
			auto& dt = pState.mDistantTile;
			dt->mPosX = pPosX;
			dt->mPosY = pPosY;
			dt->mPosZ = pPosZ;

			//If unique, add to the generator. Else, it's already in there pending!
			if (dt.unique()) {
				pGenerator->InitialiseTileHighPriority(dt);
			}
		};
	
		auto TDF = [](DOUBLE pTileSize, BasicDrawable& pDrawable, ID3D11DeviceContext* pd3dContext) -> void {
			pDrawable.DrawNoCull(pd3dContext);

			auto aabb = DirectX::XMFLOAT3((FLOAT)pTileSize/2,3000,(FLOAT)pTileSize/2);
			if (pDrawable.mCamera->testFrustumAABB(pDrawable.mState.mPosition,pDrawable.mState.mCoords,aabb)) {
				pDrawable.DrawNoCull(pd3dContext);
			}
		};

		auto TUniqueF = [](DrawableState& pState) -> BOOL {
			return pState.mDistantTile.unique();
		};

		DOUBLE tileSize = pMinTileSize;
		LodLevel<DistantTile>* prevLod = NULL;
		for (UINT i = 0; i < mNumLods; i++) {
			LodLevel<DistantTile>* ll;
			if (i == mNumLods-1) {
				//Base LOD needs to be created with a higher priority to prevent holes
				ll = new LodLevel<DistantTile>(tileSize, mTileDimensionLength, OVERLAP_SCALE, mesh, shader, pCamera, pGenerator, prevLod, TCFHP, TUFHP, TDF, TUniqueF);
			}
			else {
				ll = new LodLevel<DistantTile>(tileSize, mTileDimensionLength, OVERLAP_SCALE, mesh, shader, pCamera, pGenerator, prevLod, TCF, TUF, TDF, TUniqueF);
			}

			mLods.push_back(ll);
	
			prevLod = ll;
			tileSize*=2;
		}
	}

	{
		//Initialise city

		auto TCFHigh = [](DrawableState& pState,Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY,DOUBLE pPosZ,DOUBLE pTileSize,DrawableMesh* pMesh) -> void {
			pState.mCityTile = std::shared_ptr<CityTile>(new CityTile(pPosX,pPosY,pPosZ,pTileSize,pMesh,CITY_LOD_LEVEL_HIGH));
			pGenerator->InitialiseTile(pState.mCityTile);
		};

		auto TCFMed = [](DrawableState& pState,Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY,DOUBLE pPosZ,DOUBLE pTileSize,DrawableMesh* pMesh) -> void {
			pState.mCityTile = std::shared_ptr<CityTile>(new CityTile(pPosX,pPosY,pPosZ,pTileSize,pMesh,CITY_LOD_LEVEL_MED));
			pGenerator->InitialiseTile(pState.mCityTile);
		};

		auto TCFLow = [](DrawableState& pState,Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY,DOUBLE pPosZ,DOUBLE pTileSize,DrawableMesh* pMesh) -> void {
			pState.mCityTile = std::shared_ptr<CityTile>(new CityTile(pPosX,pPosY,pPosZ,pTileSize,pMesh,CITY_LOD_LEVEL_LOW));
			pGenerator->InitialiseTile(pState.mCityTile);
		};

		auto TCFXLow = [](DrawableState& pState,Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY,DOUBLE pPosZ,DOUBLE pTileSize,DrawableMesh* pMesh) -> void {
			pState.mCityTile = std::shared_ptr<CityTile>(new CityTile(pPosX,pPosY,pPosZ,pTileSize,pMesh,CITY_LOD_LEVEL_XLOW));
			pGenerator->InitialiseTile(pState.mCityTile);
		};
		auto TCFXXLow = [](DrawableState& pState,Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY,DOUBLE pPosZ,DOUBLE pTileSize,DrawableMesh* pMesh) -> void {
			pState.mCityTile = std::shared_ptr<CityTile>(new CityTile(pPosX,pPosY,pPosZ,pTileSize,pMesh,CITY_LOD_LEVEL_XXLOW));
			pGenerator->InitialiseTileHighPriority(pState.mCityTile);
		};

		auto TUF = [](DrawableState& pState, Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY, DOUBLE pPosZ) -> void {
			auto& dt = pState.mCityTile;
			dt->mPosX = pPosX;
			dt->mPosY = pPosY;
			dt->mPosZ = pPosZ;

			//If unique, add to the generator. Else, it's already in there pending!
			if (dt.unique()) {
				pGenerator->InitialiseTile(dt);
			}
		};

		auto TUFHP = [](DrawableState& pState, Generator* pGenerator, DOUBLE pPosX,DOUBLE pPosY, DOUBLE pPosZ) -> void {
			auto& dt = pState.mCityTile;
			dt->mPosX = pPosX;
			dt->mPosY = pPosY;
			dt->mPosZ = pPosZ;

			//If unique, add to the generator. Else, it's already in there pending!
			if (dt.unique()) {
				pGenerator->InitialiseTileHighPriority(dt);
			}
		};

		auto TDF = [](DOUBLE pTileSize, BasicDrawable& pDrawable, ID3D11DeviceContext* pd3dContext) -> void {
			auto aabb = DirectX::XMFLOAT3((FLOAT)pTileSize/2,2500,(FLOAT)pTileSize/2);
			if (pDrawable.mCamera->testFrustumAABB(pDrawable.mState.mPosition,pDrawable.mState.mCoords,aabb)) {
				pDrawable.DrawInstancedIndirect(pd3dContext);
			}
		};

		auto TUniqueF = [](DrawableState& pState) -> BOOL {
			return pState.mCityTile.unique();
		};

		DrawableShader* cityShader = pShaderManager->getDrawableShader("GBufferShader");
		DrawableMesh* cityMesh = pMeshManager->getDrawableMesh("CubeMesh");

		float cityTileDim = 8192;

		DOUBLE dimension = (pMinTileSize * pow(2,mNumLods-1) * mTileDimensionLength)/cityTileDim;

		dimension = min(dimension,32);

		//Ensure dimension is even
		dimension = ceil(dimension/2) * 2;

		cityTileDim/=16;

		mCityLods.push_back(new LodLevel<CityTile>(cityTileDim, (UINT)dimension, 1.0f/cityTileDim, cityMesh, cityShader, pCamera, pGenerator, NULL, TCFHigh, TUF, TDF, TUniqueF));

		cityTileDim*=2;

		mCityLods.push_back(new LodLevel<CityTile>(cityTileDim, (UINT)dimension, 1.0f/cityTileDim, cityMesh, cityShader, pCamera, pGenerator, mCityLods[0], TCFMed, TUF, TDF, TUniqueF));

		cityTileDim*=2;

		mCityLods.push_back(new LodLevel<CityTile>(cityTileDim, (UINT)dimension, 1.0f/cityTileDim, cityMesh, cityShader, pCamera, pGenerator, mCityLods[1], TCFLow, TUF, TDF, TUniqueF));

		cityTileDim*=2;

		mCityLods.push_back(new LodLevel<CityTile>(cityTileDim, (UINT)dimension, 1.0f/cityTileDim, cityMesh, cityShader, pCamera, pGenerator, mCityLods[2], TCFXLow, TUF, TDF, TUniqueF));

		cityTileDim*=2;

		mCityLods.push_back(new LodLevel<CityTile>(cityTileDim, (UINT)dimension, 1.0f/cityTileDim, cityMesh, cityShader, pCamera, pGenerator, mCityLods[3], TCFXXLow, TUFHP, TDF, TUniqueF));
	}
}

DistantDrawable::~DistantDrawable()
{
	for (int i = 0; i < mLods.size(); i++ ) {
		delete mLods[i];
	}
	for (int i = 0; i < mCityLods.size(); i++) {
		delete mCityLods[i];
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

	size_t num_citylods = mCityLods.size();

	for (int i = 0; i < num_citylods; i++) {
		mCityLods[i]->Update();
	}

	mCityLods[num_citylods-1]->Draw(pd3dContext);
}