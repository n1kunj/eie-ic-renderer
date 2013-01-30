#include "DXUT.h"
#include "DrawableManager.h"

DrawableManager::DrawableManager() : mDrawableVector()
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
}
