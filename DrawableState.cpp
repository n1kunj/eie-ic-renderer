#include "DXUT.h"
#include "DrawableState.h"
DrawableState::DrawableState() : mDirty(TRUE)
{
	// Initialize the world matrix
	mWorldViewMatrix = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( 0.0f, 1.0f, -5.0f, 0.0f );
	XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	this->mViewMatrix = XMMatrixLookAtLH( Eye, At, Up );

}

DrawableState::~DrawableState()
{

}

void DrawableState::update()
{
	if (mDirty) {
		return;
	}
	mDirty = FALSE;


}
