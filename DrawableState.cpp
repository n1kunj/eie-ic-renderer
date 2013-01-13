#include "DXUT.h"
#include "DrawableState.h"
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
	if (mDirty) {
		return;
	}
	mDirty = FALSE;




}
