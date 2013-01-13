#include "DXUT.h"
#include "Camera.h"

void Camera::updateMatrices(DXGI_SURFACE_DESC pSurfaceDesc)
{
	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( 0.0f, 1.0f, -5.0f, 0.0f );
	XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	this->mViewMatrix = XMMatrixLookAtLH( Eye, At, Up );

	mProjectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV2, pSurfaceDesc.Width / (FLOAT)pSurfaceDesc.Height, 0.01f, 100.0f );

	mViewProjectionMatrix = XMMatrixMultiply(mViewMatrix,mProjectionMatrix);
}
