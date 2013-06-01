function runcube()
run("cube.lua")
end

--CAMERA:setEye(0,300,0)

reset()

distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,4,7,256);

DRAWMAN:addDrawable(distDr)

shader = SHADERMAN:getDrawableShader("GBufferShader")
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