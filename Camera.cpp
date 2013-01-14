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

Camera::Camera() : mouseLook(FALSE), mMouseStart(0,0), mMouseEnd(0,0), mMouseDistanceX(0), mMouseDistanceY(0)
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

void Camera::updateCamera(XMINT2 pMouseDelta) {

		mMouseDistanceX += pMouseDelta.x;
		mMouseDistanceY += pMouseDelta.y;

		if (mMouseDistanceX > 2 * M_PI / CAMERALOOKSCALE) {
			mMouseDistanceX-= 2 * M_PI / CAMERALOOKSCALE;
		}
		else if (mMouseDistanceX < -2 * M_PI / CAMERALOOKSCALE) {
			mMouseDistanceX+= 2 * M_PI / CAMERALOOKSCALE;
		}

		//Set X axis vars
		DOUBLE curx = mMouseDistanceX;
		DOUBLE sinx = sin(CAMERALOOKSCALE*curx);
		DOUBLE cosx = cos(CAMERALOOKSCALE*curx);

		//Set Y axis vars
		DOUBLE cury = mMouseDistanceY;
		DOUBLE siny;
		DOUBLE cosy;

		//Nik's super awesome FPS style camera. Won't let you look upside down.
		if (cury >= M_PI_2 / CAMERALOOKSCALE)
		{
			mMouseDistanceY = M_PI_2 / CAMERALOOKSCALE;

			siny = 1;
			cosy = 0.0001f;
		}
		else if (cury <= -M_PI_2 / CAMERALOOKSCALE)
		{
			mMouseDistanceY = -M_PI_2 / CAMERALOOKSCALE;
			siny = -1;
			cosy = 0.0001f;
		}
		else {
			siny = sin(CAMERALOOKSCALE*cury);
			cosy = cos(CAMERALOOKSCALE*cury);
		}

		//Look angle (really a unit vector)
		DOUBLE anglex = sinx * cosy;
		DOUBLE angley = -siny;
		DOUBLE anglez = -cosx * cosy;
		DOUBLE anormal = sqrt(anglex * anglex + angley * angley + anglez * anglez);
		anglex /= anormal;
		angley /= anormal;
		anglez /= anormal;

		mLookVector = XMVectorSet((FLOAT)anglex,(FLOAT)angley,(FLOAT)anglez,0.0f);
}

Camera::~Camera()
{
}
