function runcube()
run("cube.lua")
end

--CAMERA:setEye(0,2000,0)

reset()

--distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,12,128,0,640)
--DRAWMAN:addDrawable(distDr)

 distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,12,256,0,1280)
 DRAWMAN:addDrawable(distDr)

--distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,12,256,512,1280)
--DRAWMAN:addDrawable(distDr)

distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,12,512,1024,2560)
DRAWMAN:addDrawable(distDr)

distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,12,1024,2048,5120)
DRAWMAN:addDrawable(distDr)

distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,12,2048,4096,10240)
DRAWMAN:addDrawable(distDr)

distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,12,4096,8192,20480)
DRAWMAN:addDrawable(distDr)

distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,12,8192,16384,49152)
DRAWMAN:addDrawable(distDr)



shader = SHADERMAN:getDrawableShader("GBufferShader")
--shader = SHADERMAN:getDrawableShader("DefaultShader")
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
for i=1,200 do
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