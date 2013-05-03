#include "DXUT.h"
#include "Drawable.h"
#include "DrawableMesh.h"
#include "DrawableShader.h"
#include "Camera.h"

void BasicDrawable::Draw(ID3D11DeviceContext* pd3dContext)
{
	//TODO: bounds of a given object
	if (mCamera->testFrustum(mState.mPosition,mState.mCoords, mState.getBoundingRadius()) == TRUE) {
		mState.updateMatrices();
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

void BasicDrawable::DrawInstanced( ID3D11DeviceContext* pd3dContext, BasicDrawable* pDrawableList, UINT pCount )
{
	Camera* camera = pDrawableList[0].mCamera;
	DrawableMesh* mesh = pDrawableList[0].mMesh;
	DrawableShader* shader = pDrawableList[0].mShader;

	UINT count = 0;
	DrawableState** drawers = new DrawableState*[pCount];

	for (UINT i = 0; i < pCount; i++) {
		DrawableState& state = pDrawableList[i].mState;
		if (camera->testFrustum(state.mPosition, state.mCoords, state.getBoundingRadius())) {
			state.updateMatrices();
			drawers[count] = &state;
			count++;
		}
	}

	shader->DrawInstanced(pd3dContext,mesh,drawers,camera,count);
}

void BasicDrawable::DrawInstancedIndirect(ID3D11DeviceContext* pd3dContext) {
	//if (mCamera->testFrustum(mState.mPosition,mState.mCoords, mState.getBoundingRadius()) == TRUE) {
		mState.updateMatrices();
		mShader->DrawInstancedIndirect(pd3dContext,mMesh,&mState,mCamera);
	//}
}