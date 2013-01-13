#include "DXUT.h"
#include "DrawableState.h"
DrawableState::DrawableState()
{
	// Initialize the world matrix
	mWorldViewMatrix = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( 0.0f, 1.0f, -5.0f, 0.0f );
	XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	this->mViewMatrix = XMMatrixLookAtLH( Eye, At, Up );

	// Initialize the projection matrix
	//this->mProjectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV2, pSurfaceDesc->Width / (FLOAT)pSurfaceDesc->Height, 0.01f, 100.0f );

}

DrawableState::~DrawableState()
{

}