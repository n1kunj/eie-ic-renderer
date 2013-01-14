#include "DXUT.h"
#include "DrawableState.h"

using namespace DirectX;

DrawableState::DrawableState() : mDirty(TRUE), mPosition(0,0,0), mRotation(0,0,0)
{
	// Initialize the world matrix
	mModelMatrix = XMMatrixIdentity();
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

	XMMATRIX translateMatrix = XMMatrixTranslation(mPosition.x,mPosition.y,mPosition.z);
	XMMATRIX rotateMatrix = XMMatrixRotationRollPitchYaw(mRotation.x,mRotation.y,mRotation.z);

	mModelMatrix = XMMatrixMultiply(translateMatrix, rotateMatrix);

}
