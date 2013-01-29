#include "DXUT.h"
#include "DrawableManager.h"
#include "Octree.h"

DrawableManager::DrawableManager() : mDrawableVector(), mOctreeVector()
{
	mDrawableVector.reserve(10000);
}

DrawableManager::~DrawableManager()
{
	reset();
}

void DrawableManager::addDrawable( Drawable* pDrawable )
{
	mDrawableVector.push_back(pDrawable);
	for (UINT i = 0; i < mOctreeVector.size(); i++) {
		if (mOctreeVector[i].mCamera == pDrawable->mCamera) {
			mOctreeVector[i].mOctree->add(pDrawable);
			return;
		}
	}

	OctreeStruct octreeStruct;
	octreeStruct.mCamera = pDrawable->mCamera;
	octreeStruct.mOctree = new Octree();

	octreeStruct.mOctree->add(pDrawable);

	mOctreeVector.push_back(octreeStruct);

}

//Return TRUE if successfully removed, else FALSE
BOOLEAN DrawableManager::removeDrawable( Drawable* pDrawable, BOOLEAN pDelete)
{
	for (int i = 0; i < mDrawableVector.size(); i++) {
		if (mDrawableVector[i] == pDrawable) {
			mDrawableVector[i] = NULL;
			if (pDelete) {
				SAFE_DELETE(pDrawable);
			}
			return TRUE;
		}
	}
	return FALSE;
}

void DrawableManager::Draw(ID3D11DeviceContext* pd3dContext)
{
	for (UINT i = 0; i < mDrawableVector.size(); i++) {
		Drawable* d = mDrawableVector[i];
		if (d!=NULL) {
			d->Draw(pd3dContext);
		}
	}
}

void DrawableManager::reset()
{
	for (UINT i = 0; i < mDrawableVector.size(); i++) {
		SAFE_DELETE(mDrawableVector[i]);
	}
	mDrawableVector.clear();

	for (UINT i = 0; i < mOctreeVector.size(); i++) {
		SAFE_DELETE(mOctreeVector[i].mOctree);
	}
	mOctreeVector.clear();
}
