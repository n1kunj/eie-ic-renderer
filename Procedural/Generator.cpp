#include "DXUT.h"
#include "Generator.h"
#include "../Utils/ShaderTools.h"
#include "SimplexNoise.h"
#include <algorithm>

#define ALBNORM_MAP_RESOLUTION 256
#define HEIGHT_MAP_RESOLUTION 256
#define DISTANT_CS_GROUP_DIM 16
#define DISTANT_DISPATCH_DIM (ALBNORM_MAP_RESOLUTION/DISTANT_CS_GROUP_DIM)

#define CITY_CS_GROUP_DIM 8
#define CITY_GRID_SIZE 128

__declspec(align(16)) struct GlobalTileCSCB {
	UINT numNoiseIterations;
	UINT numBiomes;
	DirectX::XMUINT2 padding;
};

__declspec(align(16)) struct DistantTileCSCB {
	DirectX::XMUINT2 bufferDim;
	DirectX::XMINT2 coords;
	UINT tileSize;
	DirectX::XMUINT3 padding;
};

__declspec(align(16)) struct CityTileCSCB {
	DirectX::XMINT2 coords;
	UINT tileSize;
	UINT lodLevel;
};

DistantTile::DistantTile(DOUBLE pPosX, DOUBLE pPosY, DOUBLE pPosZ, DOUBLE pSize, INT pPriority) {
	mPriority = pPriority;
	mPosX = pPosX;
	mPosY = pPosY;
	mPosZ = pPosZ;
	mSize = pSize;

	D3D11_TEXTURE2D_DESC desc;
	::ZeroMemory (&desc, sizeof (desc));
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
	desc.Height = ALBNORM_MAP_RESOLUTION;
	desc.Width = ALBNORM_MAP_RESOLUTION;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	mAlbedoMap.mDesc = desc;

	desc.Format = DXGI_FORMAT_R8G8B8A8_SNORM;
	mNormalMap.mDesc = desc;

	desc.MipLevels = 1;
	desc.MiscFlags = 0;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.Format = DXGI_FORMAT_R32_FLOAT;
	mHeightMap.mDesc = desc;
}

CityTile::CityTile( DOUBLE pPosX, DOUBLE pPosY, DOUBLE pPosZ, DOUBLE pSize, DrawableMesh* pMesh, CityLodLevel pCLL, INT pPriority) : mCLL(pCLL), mPriority(pPriority)
{
	mPosX = pPosX;
	mPosY = pPosY;
	mPosZ = pPosZ;
	mSize = pSize;
	{
		UINT maxObjects = MAX_OBJECTS_PER_GRID_TILE[mCLL];
		UINT numBuildings = (UINT)(mSize / CITY_GRID_SIZE);
		numBuildings *= numBuildings * maxObjects;

		auto& ib = mInstanceBuffer;
		ib.mBindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		ib.mCPUAccessFlags = 0;
		ib.mUsage = D3D11_USAGE_DEFAULT;
		ib.mElements = (UINT)numBuildings;
		ib.mDefaultUAVDesc = FALSE;
		ib.mUAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
		ib.mUAVDesc.Buffer.NumElements = (UINT)numBuildings;
		ib.mUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	}
	{
		auto& ib =  mIndirectBuffer;
		ib.mCPUAccessFlags = 0;
		ib.mUsage = D3D11_USAGE_DEFAULT;
		ib.mElements = 5;
		ib.mMiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

		//Arguments for drawinstancedindexedindirect
		mIndirectData[0] = pMesh->mNumIndices;
		mIndirectData[1] = 0; //This will be updated later
		mIndirectData[2] = 0;
		mIndirectData[3] = 0;
		mIndirectData[4] = 0;
	}
}


void Generator::Generate( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, FLOAT pMaxRuntimeSeconds )
{
	FLOAT elapsedTime = 0;

	if (!mBufferInit) {
		InitialiseBuffers(pd3dDevice,pd3dContext);
	}

	//Load in everything right at the start
	if (mInitialLoad) {
		elapsedTime = -FLT_MAX;
		mInitialLoad = FALSE;
	}

	auto cityCompare = [](CTPTR a, CTPTR b) -> BOOL {
		return a->mPriority > b->mPriority;
	};
	auto distantCompare = [](DTPTR a, DTPTR b) -> BOOL {
		return a->mPriority > b->mPriority;
	};

	std::sort(mCityQueue.begin(),mCityQueue.end(),cityCompare);
	std::sort(mTextureQueue.begin(),mTextureQueue.end(),distantCompare);

	while (elapsedTime <= pMaxRuntimeSeconds) {
		if (mTextureQueueHP.size() != 0) {
			elapsedTime += ProcessDT(pd3dDevice,pd3dContext,mTextureQueueHP);
		}
		else if (mCityQueueHP.size() != 0) {
			elapsedTime += ProcessCT(pd3dDevice,pd3dContext,mCityQueueHP);
		}
		else {
			size_t mcqSize = mCityQueue.size();
			size_t mtqSize = mTextureQueue.size();
			if (mcqSize == 0 && mtqSize == 0) {
				break;
			}

			BOOL cityFirst;
			if (mcqSize && !mtqSize) {
				cityFirst = TRUE;
			}
			else if (mtqSize && !mcqSize) {
				cityFirst = FALSE;
			}
			else {
				INT mcqp = mCityQueue.front()->mPriority + mCityHandicap;
				INT mtqp = mTextureQueue.front()->mPriority;
				if (mcqp >= mtqp) {
					cityFirst = TRUE;
				}
				else {
					cityFirst = FALSE;
				}
			}

			if (cityFirst) {
				elapsedTime += ProcessCT(pd3dDevice,pd3dContext,mCityQueue);
			}
			else {
				elapsedTime += ProcessDT(pd3dDevice,pd3dContext,mTextureQueue);
			}
		}
	}
	mFrameNumber++;
}

FLOAT Generator::ProcessDT(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, std::deque<DTPTR> &pDTqueue) {

	static bool newQuery = TRUE;
	static FLOAT sEstTime = 0.01f;
	static UINT64 sQueryFrame = 0;

	if (dtDisjoint == NULL) {
		D3D11_QUERY_DESC qdesc;
		qdesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		qdesc.MiscFlags = 0;
		pd3dDevice->CreateQuery(&qdesc, &dtDisjoint);

		qdesc.Query = D3D11_QUERY_TIMESTAMP;
		pd3dDevice->CreateQuery(&qdesc, &dtStart);
		pd3dDevice->CreateQuery(&qdesc, &dtEnd);

	}

	if (newQuery) {
		pd3dContext->Begin(dtDisjoint);
		pd3dContext->End(dtStart);
		sQueryFrame = mFrameNumber+1;
	}

	DTPTR &first = pDTqueue.front();

	if (first.unique()) {
		first.reset();
	}
	else {
		//Check if the texture already exists
		if (first->mAlbedoMap.mTexture == NULL) {
			first->mAlbedoMap.CreateTexture(pd3dDevice);
			first->mNormalMap.CreateTexture(pd3dDevice);
			first->mHeightMap.CreateTexture(pd3dDevice);
		}
		ComputeTextures(pd3dContext, *first);
		pd3dContext->GenerateMips(first->mAlbedoMap.mSRV);
		pd3dContext->GenerateMips(first->mNormalMap.mSRV);
	}
	pDTqueue.pop_front();

	if (newQuery) {
		pd3dContext->End(dtEnd);
		pd3dContext->End(dtDisjoint);
		newQuery = FALSE;
	}

	if (sQueryFrame <= mFrameNumber && !newQuery) {
		UINT64 end;
		if ( S_OK == pd3dContext->GetData(dtEnd, &end, sizeof(UINT64), 0)) {
			D3D11_QUERY_DATA_TIMESTAMP_DISJOINT djData;
			if (S_OK == pd3dContext->GetData(dtDisjoint, &djData, sizeof(djData),0)){
				UINT64 start;
				if ( S_OK == pd3dContext->GetData(dtStart, &start, sizeof(UINT64),0)){

					FLOAT time = ((FLOAT)(end-start))/djData.Frequency;
					sEstTime = 0.8f * sEstTime + 0.2f * time;

					newQuery = TRUE;
				}
			}
		}
		else {
			sQueryFrame = mFrameNumber + 1;
		}
	}

	return sEstTime;
}

FLOAT Generator::ProcessCT( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, std::deque<CTPTR> &pCTqueue)
{
	static bool newQuery = TRUE;
	static FLOAT sEstTimePerBuilding = 0.001f;
	static UINT64 sQueryFrame = 0;

	if (ctDisjoint == NULL) {
		D3D11_QUERY_DESC qdesc;
		qdesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		qdesc.MiscFlags = 0;
		pd3dDevice->CreateQuery(&qdesc, &ctDisjoint);

		qdesc.Query = D3D11_QUERY_TIMESTAMP;
		pd3dDevice->CreateQuery(&qdesc, &ctStart);
		pd3dDevice->CreateQuery(&qdesc, &ctEnd);

	}

	if (newQuery) {
		pd3dContext->Begin(ctDisjoint);
		pd3dContext->End(ctStart);
		sQueryFrame = mFrameNumber+1;
	}


	CTPTR &first = pCTqueue.front();

	if (first.unique()) {
		first.reset();
	}
	else {
		if (first->mInstanceBuffer.mBuffer == NULL) {
			first->mInstanceBuffer.CreateBuffer(pd3dDevice);
			first->mIndirectBuffer.CreateBuffer(pd3dDevice,first->mIndirectData);
		}
		ComputeCity(pd3dContext,*first);
	}
	FLOAT numBuildings = (FLOAT)first->mSize/CITY_GRID_SIZE;

	pCTqueue.pop_front();

	if (newQuery) {
		pd3dContext->End(ctEnd);
		pd3dContext->End(ctDisjoint);
		newQuery = FALSE;
	}

	if (sQueryFrame <= mFrameNumber && !newQuery) {
		UINT64 end;
		if ( S_OK == pd3dContext->GetData(ctEnd, &end, sizeof(UINT64), 0)) {
			D3D11_QUERY_DATA_TIMESTAMP_DISJOINT djData;
			if (S_OK == pd3dContext->GetData(ctDisjoint, &djData, sizeof(djData),0)){
				UINT64 start;
				if ( S_OK == pd3dContext->GetData(ctStart, &start, sizeof(UINT64),0)){

					FLOAT time = ((FLOAT)(end-start))/djData.Frequency;
					sEstTimePerBuilding = 0.8f * sEstTimePerBuilding + 0.2f * time/numBuildings;

					newQuery = true;
				}
			}
		}
		else {
			sQueryFrame = mFrameNumber + 1;
		}
	}

	return sEstTimePerBuilding*numBuildings;
}

void Generator::ComputeTextures(ID3D11DeviceContext* pd3dContext, DistantTile &pDT )
{
	if (!mCompiled) {
		return;
	}

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dContext->Map(mCSCBDistant,0,D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	DistantTileCSCB* cscb = (DistantTileCSCB*)MappedResource.pData;
	cscb->bufferDim = DirectX::XMUINT2(ALBNORM_MAP_RESOLUTION, ALBNORM_MAP_RESOLUTION);
	cscb->coords = DirectX::XMINT2((INT)pDT.mPosX,(INT)pDT.mPosZ);
	cscb->tileSize = (UINT)pDT.mSize;

	pd3dContext->Unmap(mCSCBDistant,0);

	ID3D11Buffer* cbs[2] = {mCSCBGlobal,mCSCBDistant};

	pd3dContext->CSSetConstantBuffers(0,2,cbs);

	ID3D11UnorderedAccessView* uavs[3] = {pDT.mAlbedoMap.mUAV,
		pDT.mNormalMap.mUAV,pDT.mHeightMap.mUAV};

	pd3dContext->CSSetUnorderedAccessViews(0,3,uavs,0);

	ID3D11ShaderResourceView* srvs[5] = {mSimplexBuffer.mSRV,mScalesBuffer.mSRV,mCoeffsBuffer.mSRV,mColourCityBuffer.mSRV,mSpecPowBuffer.mSRV};

	pd3dContext->CSSetShaderResources(0,5,srvs);

	pd3dContext->CSSetShader(mCSDistant,0,0);
	pd3dContext->Dispatch(DISTANT_DISPATCH_DIM,DISTANT_DISPATCH_DIM,1);

	ID3D11UnorderedAccessView* nulluavs[3] = {NULL,NULL,NULL};
	pd3dContext->CSSetUnorderedAccessViews(0,3,nulluavs,0);
	ID3D11ShaderResourceView* nullsrv[5] = {NULL,NULL,NULL,NULL,NULL};
	pd3dContext->CSSetShaderResources(0,5,nullsrv);
}

void Generator::ComputeCity( ID3D11DeviceContext* pd3dContext, CityTile &pCT )
{
	if (!mCompiled) {
		return;
	}

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dContext->Map(mCSCBCity,0,D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CityTileCSCB* cscb = (CityTileCSCB*)MappedResource.pData;
	cscb->coords = DirectX::XMINT2((INT)pCT.mPosX,(INT)pCT.mPosZ);
	cscb->tileSize = (UINT)pCT.mSize;
	cscb->lodLevel = (UINT)pCT.mCLL;
	pd3dContext->Unmap(mCSCBCity,0);

	ID3D11Buffer* cbs[2] = {mCSCBGlobal,mCSCBCity};

	pd3dContext->CSSetConstantBuffers(0,2,cbs);
	const UINT count = 0;
	pd3dContext->CSSetUnorderedAccessViews(0,1,&pCT.mInstanceBuffer.mUAV,&count);

	ID3D11ShaderResourceView* srvs[5] = {mSimplexBuffer.mSRV,mScalesBuffer.mSRV,mCoeffsBuffer.mSRV,mColourCityBuffer.mSRV,mSpecPowBuffer.mSRV};

	pd3dContext->CSSetShaderResources(0,5,srvs);

	pd3dContext->CSSetShader(mCSCity,0,0);

	UINT dwidth = (UINT)(pCT.mSize/(CITY_CS_GROUP_DIM * CITY_GRID_SIZE));

	pd3dContext->Dispatch(dwidth,dwidth,1);

	pd3dContext->CopyStructureCount(pCT.mIndirectBuffer.mBuffer,4,pCT.mInstanceBuffer.mUAV);

	ID3D11UnorderedAccessView* nulluavs[1] = {NULL};
	pd3dContext->CSSetUnorderedAccessViews(0,1,nulluavs,0);

	ID3D11ShaderResourceView* nullsrv[5] = {NULL,NULL,NULL,NULL,NULL};
	pd3dContext->CSSetShaderResources(0,5,nullsrv);
}


HRESULT Generator::OnD3D11CreateDevice( ID3D11Device* pd3dDevice )
{
	OnD3D11DestroyDevice();
	HRESULT hr;

	//Compile CS
	{
		ID3DBlob* pCSBlob = NULL;
		V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\GeneratorShader.fx", "CSDistantTile", "cs_5_0", &pCSBlob ));

		//Create the compute shader
		//If fails, releases pCSBlob.
		V_RELEASE_IF_RETURN(pCSBlob,pd3dDevice->CreateComputeShader( pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), NULL, &mCSDistant ));
	}

	//Create CSCityPass High
	{
		ID3DBlob* pCSBlob = NULL;
		V_RETURN(ShaderTools::CompileShaderFromFile( L"Shaders\\GeneratorShader.fx", "CSCityTile", "cs_5_0", &pCSBlob ));
		V_RELEASE_IF_RETURN(pCSBlob,pd3dDevice->CreateComputeShader( pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), NULL, &mCSCity ));
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.ByteWidth = sizeof(DistantTileCSCB);
	V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &mCSCBDistant ));

	bd.ByteWidth = sizeof(CityTileCSCB);
	V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &mCSCBCity ));

	bd.ByteWidth = sizeof(GlobalTileCSCB);
	V_RETURN(pd3dDevice->CreateBuffer(&bd, NULL, &mCSCBGlobal));

	mCompiled = TRUE;
	return S_OK;
}

Generator::Generator( MessageLogger* pLogger ) : mCompiled(FALSE),mCSDistant(NULL),mCSCity(NULL), mCSCBDistant(NULL), mCSCBCity(NULL), mCSCBGlobal(NULL), mLogger(pLogger), mBufferInit(FALSE), mInitialLoad(FALSE), mFrameNumber(0)
{
	dtDisjoint = NULL;
	dtStart = NULL;
	dtEnd = NULL;

	ctDisjoint = NULL;
	ctStart = NULL;
	ctEnd = NULL;

	//if x0 > y0, use right 16 bits, else use left 16 bits
	UINT i1 = 0;
	UINT j1 = 1;

	for (int ii = 0; ii < 256; ii++) {
		for (int jj = 0; jj < 256; jj++) {
			mSimplex2DLUT[ii][jj]=0;

			for (int its = 0; its < 2; its++) {
				i1 = !i1;
				j1 = !j1;

				UINT gi0 = perm[ii+perm[jj]] % 12;
				UINT gi1 = perm[ii+i1+perm[jj+j1]] % 12;
				UINT gi2 = perm[ii+1+perm[jj+1]] % 12;

				UINT x0 = grad3[gi0][0]+1;
				UINT y0 = grad3[gi0][1]+1;
				UINT x1 = grad3[gi1][0]+1;
				UINT y1 = grad3[gi1][1]+1;
				UINT x2 = grad3[gi2][0]+1;
				UINT y2 = grad3[gi2][1]+1;

				UINT outval = 0;
				outval|=x0;
				outval|=y0<<2;
				outval|=x1<<4;
				outval|=y1<<6;
				outval|=x2<<8;
				outval|=y2<<10;
				mSimplex2DLUT[ii][jj]|= (outval << (16*its));
			}
		}
	}
	mSimplexBuffer.mBindFlags = D3D11_BIND_SHADER_RESOURCE;
	mSimplexBuffer.mUsage = D3D11_USAGE_IMMUTABLE;
	mSimplexBuffer.mElements = 256*256;

	mScalesBuffer.mBindFlags = D3D11_BIND_SHADER_RESOURCE;
	mScalesBuffer.mUsage = D3D11_USAGE_IMMUTABLE;

	mCoeffsBuffer.mBindFlags = D3D11_BIND_SHADER_RESOURCE;
	mCoeffsBuffer.mUsage = D3D11_USAGE_IMMUTABLE;

	mColourCityBuffer.mBindFlags = D3D11_BIND_SHADER_RESOURCE;
	mColourCityBuffer.mUsage = D3D11_USAGE_IMMUTABLE;

	mSpecPowBuffer.mBindFlags = D3D11_BIND_SHADER_RESOURCE;
	mSpecPowBuffer.mUsage = D3D11_USAGE_IMMUTABLE;

	setNoiseBiomeCount(12,9);
	mCityHandicap = 0;
}

void Generator::InitialiseBuffers(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext)
{
	mBufferInit = TRUE;

	mSimplexBuffer.CreateBuffer(pd3dDevice,mSimplex2DLUT);

	mScalesBuffer.mElements = (UINT)mScalesData.size();
	mScalesBuffer.CreateBuffer(pd3dDevice,&mScalesData[0]);

	mCoeffsBuffer.mElements = (UINT)mCoeffsData.size();
	mCoeffsBuffer.CreateBuffer(pd3dDevice,&mCoeffsData[0]);

	mColourCityBuffer.mElements = (UINT)mColourCityData.size();
	mColourCityBuffer.CreateBuffer(pd3dDevice,&mColourCityData[0]);

	mSpecPowBuffer.mElements = (UINT)mSpecPowData.size();
	mSpecPowBuffer.CreateBuffer(pd3dDevice,&mSpecPowData[0]);

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dContext->Map(mCSCBGlobal,0,D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	GlobalTileCSCB* cscb = (GlobalTileCSCB*)MappedResource.pData;
	cscb->numBiomes = mNumBiomes;
	cscb->numNoiseIterations = mNoiseIterations;
	pd3dContext->Unmap(mCSCBGlobal,0);
}

UINT Generator::GetMinCityTileDim()
{
	return CITY_CS_GROUP_DIM*CITY_GRID_SIZE;
}
