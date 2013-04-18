#include "DXUT.h"
#include "DistantDrawable.h"
#include "../ShaderManager.h"
#include "../Camera.h"
#include "../MeshManager.h"
#include "Generator.h"

#define SIGNUM(X) ((X > 0) ? 1 : ((X < 0) ? -1 : 0))

DistantDrawable::DistantDrawable( Camera* pCamera, ShaderManager* pShaderManager, MeshManager* pMeshManager, Generator* pGenerator, UINT pTileDimensionLength, DOUBLE pTileSize, DOUBLE pMinDrawDistance, DOUBLE pMaxDrawDistance) : Drawable(pCamera)
{
	mGenerator = pGenerator;

	mTileDimensionLength = pTileDimensionLength;
	mNumTiles = pTileDimensionLength*pTileDimensionLength;
	mTileSize = pTileSize;
	mMeshScale = pTileSize;
	mMinDrawDistance = pMinDrawDistance;
	mMaxDrawDistance = pMaxDrawDistance;

	mShader = pShaderManager->getDrawableShader("DistantGBufferShader");
	mDrawables.reserve(mNumTiles);

	DrawableMesh* mesh = pMeshManager->getDrawableMesh("Plane16");

	DOUBLE posx = 0 + mTileSize/2;
	DOUBLE posy = 0;
	DOUBLE posz = 0 + mTileSize/2;
	//TODO pos should always start around 0,0,0 then be shifted around in update

	//Prevents lots of casting later
	INT tdl = mTileDimensionLength;

	INT tilex = 0;
	INT tilez = 0;

	for (UINT i = 0; i < mNumTiles; i++) {
		mDrawables.push_back(BasicDrawable(mesh,mShader,pCamera));
		DOUBLE xLoc = posx + (tilex - tdl/2) * mTileSize;
		DOUBLE yLoc = posy;
		DOUBLE zLoc = posz + (tilez - tdl/2) * mTileSize;
		mDrawables[i].mState.setPosition(xLoc, yLoc, zLoc);
		mDrawables[i].mState.mScale = DirectX::XMFLOAT3((FLOAT)mMeshScale, (FLOAT)mMeshScale, (FLOAT)mMeshScale);
		mDrawables[i].mState.mDistantTextures = std::shared_ptr<DistantTextures>(new DistantTextures(xLoc,yLoc,zLoc, mTileSize));

		mGenerator->InitialiseDistantTile(mDrawables[i].mState.mDistantTextures);

		tilex++;
		if (tilex == tdl) {
			tilez++;
			tilex = 0;
		}
	}

	Update();
}

DistantDrawable::~DistantDrawable()
{
}

void DistantDrawable::Update() {
	DOUBLE oldX = mStickyCamX;
	DOUBLE oldY = mStickyCamY;
	DOUBLE oldZ = mStickyCamZ;

	DOUBLE camX = mCamera->getEyeX();
	DOUBLE camY = mCamera->getEyeY();
	DOUBLE camZ = mCamera->getEyeZ();

	if (camX >= 0) {
		mStickyCamX = mTileSize * floor(camX/mTileSize);
	}
	else {
		mStickyCamX = mTileSize * ceil(camX/mTileSize);
	}

	if (camY >= 0) {
		mStickyCamY = mTileSize * floor(camY/mTileSize);
	}
	else {
		mStickyCamY = mTileSize * ceil(camY/mTileSize);
	}

	if (camZ >= 0) {
		mStickyCamZ = mTileSize * floor(camZ/mTileSize);
	}
	else {
		mStickyCamZ = mTileSize * ceil(camZ/mTileSize);
	}

	//No need to be moving stuff around if the sticky cam hasn't moved.
	if (oldX == mStickyCamX && oldY == mStickyCamY && oldZ == mStickyCamZ) {
		return;
	}

	DOUBLE shiftDist = mTileSize * mTileDimensionLength;

	DOUBLE maxDist = shiftDist/2;


	for (int i = 0; i < mDrawables.size(); i++) {

		DrawableState& state = mDrawables[i].mState;

		BOOL changed = FALSE;

		DOUBLE posX = state.getPosX();
		DOUBLE posY = state.getPosY();
		DOUBLE posZ = state.getPosZ();

		DOUBLE distX = mStickyCamX - posX;
		DOUBLE distZ = mStickyCamZ - posZ;

		INT rollX = SIGNUM(distX) * (INT) floor( (abs(distX) + maxDist) / shiftDist );
		if (rollX!=0) {
			posX += rollX * shiftDist;
			state.setPosition(posX,posY,posZ);
			changed = TRUE;
		}

		INT rollZ = SIGNUM(distZ) * (INT) floor( (abs(distZ) + maxDist) / shiftDist );
		if (rollZ!=0) {
			posZ += rollZ * shiftDist;
			state.setPosition(posX,posY,posZ);
			changed = TRUE;
		}
		
		if (changed) {
			auto& dt = state.mDistantTextures;
			dt->mPosX = posX;
			dt->mPosY = posY;
			dt->mPosZ = posZ;

			//If unique, add to the generator. Else, it's already in there pending!
			if (dt.unique()) {
				mGenerator->InitialiseDistantTile(dt);
			}
		}
	}
}


void DistantDrawable::Draw( ID3D11DeviceContext* pd3dContext )
{

	Update();

	DOUBLE mindd = mMinDrawDistance;
	DOUBLE maxdd = mMaxDrawDistance;

	for (int i = 0; i < mDrawables.size(); i++) {

		DrawableState& state = mDrawables[i].mState;

		DOUBLE posX = state.getPosX();
		//DOUBLE posY = state.getPosY();
		DOUBLE posZ = state.getPosZ();

		//if ( (posX > 0 && ((INT)posX/ts)%2 == 1) || 
		//	(posX < 0 && ((INT)posX/ts)%2 == 0) ) {
		//		posX = posX - ts;
		//}

		//if ( (posZ > 0 && ((INT)posZ/ts)%2 == 1) || 
		//	(posZ < 0 && ((INT)posZ/ts)%2 == 0) ) {
		//		posZ = posZ - ts;
		//}

		DOUBLE distX = abs(posX - mStickyCamX);
		//DOUBLE distY = abs(posY - mStickyCamY);
		DOUBLE distZ = abs(posZ - mStickyCamZ);

		//If the texture hasn't been created yet, don't draw
		if (!mDrawables[i].mState.mDistantTextures.unique()) {
			continue;
		}

		//TODO: deal with y height
		if ((distX < maxdd && distZ < maxdd) && (distX >= mindd || distZ >= mindd)) {
			mDrawables[i].Draw(pd3dContext);
		}
	}
}
