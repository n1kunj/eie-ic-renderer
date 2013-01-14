#include "DXUT.h"
#include "Camera.h"
#include <windowsx.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define CAMERALOOKSCALE 0.005

//TODO: make camera be updated on message rather than on frame

void Camera::update(DXGI_SURFACE_DESC pSurfaceDesc)
{
	XMVECTOR lookAt = XMVectorAdd(mEye,mLookVector);

	this->mViewMatrix = XMMatrixLookAtLH( mEye, lookAt, mUp );

	mProjectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV2, pSurfaceDesc.Width / (FLOAT)pSurfaceDesc.Height, 0.01f, 100.0f );

	mViewProjectionMatrix = XMMatrixMultiply(mViewMatrix,mProjectionMatrix);
}

Camera::Camera() : mouseLook(FALSE), mMouseStart((SHORT)0,(SHORT)0), mMouseEnd((SHORT)0,(SHORT)0), mMouseDistanceX(0), mMouseDistanceY(0)
{
	// Initialize the view matrix
	mEye = XMVectorSet( 0.0f, 2.0f, 5.0f, 0.0f );
	mLookVector = XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );
	mUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
}

LRESULT Camera::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	*pbNoFurtherProcessing = TRUE;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		{
			mouseLook = TRUE;
			if (mMouseStart.x == mMouseEnd.x && mMouseStart.y == mMouseEnd.y) {
				mMouseStart = XMSHORT2((SHORT)GET_X_LPARAM(lParam),(SHORT)GET_Y_LPARAM(lParam));
				mMouseEnd = mMouseStart;
			}
			else {
				SHORT deltaX = mMouseEnd.x - mMouseStart.x;
				SHORT deltaY = mMouseEnd.y - mMouseEnd.y;

				mMouseStart = XMSHORT2((SHORT) (GET_X_LPARAM(lParam) - deltaX),(SHORT) (GET_Y_LPARAM(lParam) - deltaY) );
				mMouseEnd = XMSHORT2((SHORT)GET_X_LPARAM(lParam),(SHORT)GET_Y_LPARAM(lParam));
			}
			break;
		}
	case WM_LBUTTONUP:
		{
			mouseLook = FALSE;
			break;
		}
	case WM_MOUSEMOVE:
		{
			if (mouseLook == TRUE) {
				mMouseEnd = XMSHORT2((SHORT)GET_X_LPARAM(lParam),(SHORT)GET_Y_LPARAM(lParam));
				XMSHORT2 mouseDelta = XMSHORT2((SHORT) (mMouseEnd.x - mMouseStart.x),(SHORT) (mMouseEnd.y - mMouseStart.y));
				updateCamera(mouseDelta);
			}
			break;
		}
	default:
		{
			*pbNoFurtherProcessing = FALSE;
			break;
		}
	}

	return 0;
}

void Camera::updateCamera(XMSHORT2 pMouseDelta) {

		mMouseStart = mMouseEnd;
		mMouseEnd = XMSHORT2((SHORT)0,(SHORT)0);

		mMouseDistanceX += pMouseDelta.x;
		mMouseDistanceY += pMouseDelta.y;

		//Set X axis vars
		DOUBLE curx = mMouseDistanceX;
		DOUBLE sinx = sin(CAMERALOOKSCALE*(DOUBLE)curx);
		DOUBLE cosx = cos(CAMERALOOKSCALE*(DOUBLE)curx);

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
