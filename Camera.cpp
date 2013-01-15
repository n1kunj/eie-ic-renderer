#include "DXUT.h"
#include "Camera.h"
#include <windowsx.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define CAMERALOOKSCALE 0.005

using namespace DirectX;

void Camera::update(DXGI_SURFACE_DESC pSurfaceDesc)
{
	XMVECTOR lookAt = XMVectorAdd(mEye,mLookVector);

	this->mViewMatrix = XMMatrixLookAtLH( mEye, lookAt, mUp );

	mProjectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV2, pSurfaceDesc.Width / (FLOAT)pSurfaceDesc.Height, 0.01f, 100.0f );

	mViewProjectionMatrix = XMMatrixMultiply(mViewMatrix,mProjectionMatrix);
}

Camera::Camera() : mouseLook(FALSE), mMouseStart(0,0), mMouseEnd(0,0), mMoveDistanceX(0), mMoveDistanceY(0)
{
	// Initialize the view matrix
	mEye = XMVectorSet( 0.0f, 2.0f, 5.0f, 0.0f );
	mLookVector = XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );
	mUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
}

LRESULT Camera::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		{
			mouseLook = TRUE;
			SetCapture( hWnd );
			mMouseStart = XMINT2(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
			*pbNoFurtherProcessing = TRUE;
			break;
		}
	case WM_LBUTTONUP:
		{
			if (mouseLook == TRUE) {
				ReleaseCapture();
				mouseLook = FALSE;
				*pbNoFurtherProcessing = TRUE;
			}
			break;
		}
	case WM_MOUSEMOVE:
		{
			if (mouseLook == TRUE) {
				mMouseEnd = XMINT2(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
				XMINT2 mouseDelta = XMINT2(mMouseStart.x - mMouseEnd.x, mMouseEnd.y - mMouseStart.y);
				mMouseStart = mMouseEnd;
				mMouseEnd = XMINT2(0,0);

				updateCamera(mouseDelta);
				*pbNoFurtherProcessing = TRUE;
			}
			break;
		}
	case WM_CAPTURECHANGED:
		{
			mouseLook = FALSE;
		}
	}

	return 0;
}

void Camera::updateCamera(XMINT2 pMoveDelta) {

		mMoveDistanceX += pMoveDelta.x;
		mMoveDistanceY += pMoveDelta.y;

		if (mMoveDistanceX > 2 * M_PI / CAMERALOOKSCALE) {
			mMoveDistanceX-= 2 * M_PI / CAMERALOOKSCALE;
		}
		else if (mMoveDistanceX < -2 * M_PI / CAMERALOOKSCALE) {
			mMoveDistanceX+= 2 * M_PI / CAMERALOOKSCALE;
		}

		//Set X axis vars
		DOUBLE curX = CAMERALOOKSCALE * mMoveDistanceX;
		DOUBLE sinX = sin(curX);
		DOUBLE cosX = cos(curX);

		//Set Y axis vars
		DOUBLE curY = CAMERALOOKSCALE * mMoveDistanceY;
		DOUBLE sinY;
		DOUBLE cosY;

		//Nik's super awesome FPS style camera. Won't let you look upside down.
		if (curY >= M_PI_2)
		{
			mMoveDistanceY = M_PI_2 / CAMERALOOKSCALE;

			sinY = 1;
			cosY = 0.0001f;
		}
		else if (curY <= -M_PI_2)
		{
			mMoveDistanceY = -M_PI_2 / CAMERALOOKSCALE;
			sinY = -1;
			cosY = 0.0001f;
		}
		else {
			sinY = sin(curY);
			cosY = cos(curY);
		}

		//Look angle (really a unit vector)
		DOUBLE angleX = sinX * cosY;
		DOUBLE angleY = -sinY;
		DOUBLE angleZ = -cosX * cosY;
		DOUBLE angleMag = sqrt(angleX * angleX + angleY * angleY + angleZ * angleZ);
		angleX /= angleMag;
		angleY /= angleMag;
		angleZ /= angleMag;

		mLookVector = XMVectorSet((FLOAT)angleX,(FLOAT)angleY,(FLOAT)angleZ,0.0f);
}

Camera::~Camera()
{
}
