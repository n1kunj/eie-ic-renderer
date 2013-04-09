#include "DXUT.h"
#include "Drawable.h"
#include "DrawableMesh.h"
#include "DrawableShader.h"
#include "Camera.h"

void BasicDrawable::Draw(ID3D11DeviceContext* pd3dContext)
{
	//TODO: bounds of a given object
	if (mCamera->testFrustum(mState.mPosition,mState.mCoords,10) == TRUE) {
		mState.updateMatrices(mCamera);
		mShader->DrawMesh(pd3dContext,mMesh,&mState,mCamera);
	}
}

BasicDrawable::BasicDrawable(DrawableMesh* pMesh, DrawableShader* pShader, Camera* pCamera)
	: mState(), mMesh(pMesh), mShader(pShader), Drawable(pCamera)
{

}

BasicDrawable::BasicDrawable( const BasicDrawable& copy ) : Drawable(copy.mCamera)
{
	mState = copy.mState;
	mMesh = copy.mMesh;
	mShader = copy.mShader;
}

BasicDrawable::~BasicDrawable()
{

}
