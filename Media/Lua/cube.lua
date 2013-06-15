function runcube()
run("cube.lua")
end

--CAMERA:setEye(0,300,0)

reset()

numNoises = 12
numBiomes = 9

GENERATOR:setNoiseBiomeCount(numNoises,numBiomes);

scales = {150000, 30000, 12800,	640,
		320, 160, 80, 40,
		20, 10, 5, 2.5}

for i=1,numNoises do
GENERATOR:setScalesData(i-1,scales[i]);
end

coeffs = {
0,0,2,1,0.5,0.5,0.5,0.5,0.5,0.5,0.25,0.25,

0,0,0,0,0,0,0,0,0,0,0,0,
128,0,0,0,0,0,0,0,0,0,0,0,
256,0,0,0,0,0,0,0,0,0,0,0,
128,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,

-- 0,0,0,0,0,0.25,0,0,0,0.25,0.25,0,
-- 2048,512,128,64,32,16,8,4,2,1,0.5f,0.25f,
-- 2048,512,128,64,32,16,8,4,2,1,0.5f,0.25f,
-- 2048,512,128,64,32,16,8,4,2,1,0.5f,0.25f,
-- 0,0,0,0,0,0.25,0,0,0,0.25,0.25,0,

0,0,2,1,0.5,0.5,0.5,0.5,0.5,0.5,0.25,0.25,
0,0,2,1,0.5,0.5,0.5,0.5,0.5,0.5,0.25,0.25,
0,0,2,1,0.5,0.5,0.5,0.5,0.5,0.5,0.25,0.25
};

for i=1,numNoises*numBiomes do
GENERATOR:setCoeffsData(i-1,coeffs[i]);
end	

cols = {
0,0,0.5,1,

0.9,0.9,0.9,0,
0.9,0.9,0.9,0,
0.9,0.9,0.9,0,
0.9,0.9,0.9,0,
0.9,0.9,0.9,0,

-- 0.5,0.5,0.5,1,
-- 0.5,0.5,0.5,1,
-- 0.5,0.5,0.5,1,
-- 0.5,0.5,0.5,1,
-- 0.5,0.5,0.5,1,

0,0,0.5,1,
0,0,0.4,1,
0,0,0.3,1,
};

for i=0,numBiomes-1 do
GENERATOR:setColourCityData(i,cols[i*4+1],cols[i*4+2],cols[i*4+3],cols[i*4+4]);
end

spec = {
20,1,
128,0.1,
128,0.1,
128,0.1,
128,0.1,
128,0.1,
20,1,
20,1,
20,1,
};

for i=0,numBiomes-1 do
GENERATOR:setSpecPowData(i,spec[i*2+1],spec[i*2+2]);
end

--distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,2,1,2048*4);
distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,6,9,170);
--distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,16,7,256);
--distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,4,1,2048*32);
--distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,6,3,256);


DRAWMAN:addDrawable(distDr)

shader = SHADERMAN:getDrawableShader("CityGBufferShader")
mesh = MESHMAN:getDrawableMesh("CubeMesh")

lightSrc = BasicDrawable(mesh,shader,CAMERA)

state = lightSrc.mState

state:setPosition(5,3,2)

state.mAmbientColour.x = 99999
state.mAmbientColour.y = 99999
state.mAmbientColour.z = 99999

state.mScale.x = 0.2
state.mScale.y = 0.2
state.mScale.z = 0.2

DRAWMAN:addDrawable(lightSrc)
math.randomseed(100)
for i=1,1 do
cubeptr = BasicDrawable(mesh,shader,CAMERA)

a = math.random()
b = math.random()
c = math.random()

posx = 100 * a - 50
posy = 100 * b - 50
posz = 100 * c - 50

cubeptr.mState:setPosition(posx,posy,posz)

dcol = cubeptr.mState.mDiffuseColour

dcol.x = a
dcol.y = b
dcol.z = c

cubeptr.mState.mSpecularExponent = 1000 * math.random()

cubeptr.mState.mSpecularAmount = 3 * math.random()


DRAWMAN:addDrawable(cubeptr)
end