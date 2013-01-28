#include "DXUT.h"
#include "Drawable.h"
#include "DrawableMesh.h"
#include "DrawableShader.h"
#include "Camera.h"

void BasicDrawable::Draw(ID3D11DeviceContext* pd3dContext)
{
	//TODO: bounds
	if (mCamera->testFrustum(mState.mPosition,10) == TRUE) {
		mState.updateMatrices();
		mShader->DrawMesh(pd3dContext,mMesh,&mState,mCamera);
	}
}

BasicDrawable::BasicDrawable(DrawableMesh* pMesh, DrawableShader* pShader, Camera* pCamera)
	: mState(), mMesh(pMesh), mShader(pShader), mCamera(pCamera)
{

}

BasicDrawable::BasicDrawable( const BasicDrawable& copy )
{
	mState = copy.mState;
	mMesh = copy.mMesh;
	mShader = copy.mShader;
	mCamera = copy.mCamera;
}

BasicDrawable::~BasicDrawable()
{

}
