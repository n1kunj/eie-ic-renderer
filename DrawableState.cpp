#include "DXUT.h"
#include "DrawableState.h"

using namespace DirectX;

DrawableState::DrawableState()
	: mDirty(TRUE), mPosition(0,0,0), mRotation(0,0,0),
	mScale(1.0f,1.0f,1.0f),	mDiffuseColour(1.0f,0.00f,0.50f),
	mSpecularColour(0.0f,1.0f,0.0f), mAmbientColour(1.0f,0.50f,0.50f),
	mSpecularExponent(100.0f)
{
	// Initialize the world matrix
	mModelMatrix = XMMatrixIdentity();
	mTile.x = 0;
	mTile.y = 0;
	mTile.z = 0;
}

DrawableState::~DrawableState()
{

}

void DrawableState::updateMatrices()
{
	if (mDirty == FALSE) {
		return;
	}
	mDirty = FALSE;

	XMMATRIX scaleMatrix = XMMatrixScaling(mScale.x,mScale.y,mScale.z);
	XMMATRIX rotateMatrix = XMMatrixRotationRollPitchYaw(mRotation.x,mRotation.y,mRotation.z);
	XMMATRIX transformMatrix = XMMatrixMultiply(scaleMatrix,rotateMatrix);
	XMMATRIX translateMatrix = XMMatrixTranslation(mPosition.x,mPosition.y,mPosition.z);
	mModelMatrix = XMMatrixMultiply(transformMatrix, translateMatrix);
}
