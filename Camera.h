#pragma once
#ifndef CAMERA_H
#define CAMERA_H
//#include <xnamath.h>
#include <DirectXMath.h>

//TODO: DirectXMath

class Camera {
public:
	XMVECTOR mEye;
	XMVECTOR mLookVector;
	XMVECTOR mUp;
	XMMATRIX mViewMatrix;
	XMMATRIX mProjectionMatrix;
	XMMATRIX mViewProjectionMatrix;

	void update(DXGI_SURFACE_DESC pSurfaceDesc);

	LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
		void* pUserContext );

	Camera();
	~Camera();

private:
	XMSHORT2 mMouseStart;
	XMSHORT2 mMouseEnd;
	DOUBLE mMouseDistanceX;
	DOUBLE mMouseDistanceY;

	boolean mouseLook;

	Camera(const Camera& camera);
	void updateCamera(XMSHORT2 pMouseDelta);
};

#endif