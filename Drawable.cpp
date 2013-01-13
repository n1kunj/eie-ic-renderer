#include "DXUT.h"
#include "Drawable.h"
#include "DrawableMesh.h"
#include "DrawableShader.h"
#include "Camera.h"

void Drawable::Draw(ID3D11DeviceContext* pd3dContext)
{
	assert(mShader!=NULL);
	assert(mMesh!=NULL);
	assert(mCamera!=NULL);
	mShader->DrawMesh(pd3dContext,mMesh,&mState,mCamera);
}

Drawable::Drawable(DrawableMesh* pMesh, DrawableShader* pShader, Camera* pCamera)
	: mState(), mMesh(pMesh), mShader(pShader), mCamera(pCamera)
{

}

Drawable::~Drawable()
{

}
