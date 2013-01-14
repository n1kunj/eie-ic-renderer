#pragma once
#ifndef CAMERA_H
#define CAMERA_H
//#include <xnamath.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>

//TODO: DirectXMath
using namespace DirectX;

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
	XMINT2 mMouseStart;
	XMINT2 mMouseEnd;
	DOUBLE mMouseDistanceX;
	DOUBLE mMouseDistanceY;

	boolean mouseLook;

	Camera(const Camera& camera);
	void updateCamera(XMINT2 pMouseDelta);
};

#endif