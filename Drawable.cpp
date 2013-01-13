#include "DXUT.h"
#include "Drawable.h"
#include "DrawableMesh.h"
#include "DrawableShader.h"

void Drawable::Draw(ID3D11DeviceContext* pd3dContext)
{
	assert(mShader!=NULL);
	assert(mMesh!=NULL);
	mShader->DrawMesh(pd3dContext,mMesh,&mState);
}

Drawable::Drawable(DrawableMesh* pMesh, DrawableShader* pShader)
	: mState(), mMesh(pMesh), mShader(pShader)
{

}

Drawable::~Drawable()
{

}
