function runcube()
run("cube.lua")
end

reset()

lightSrc = BasicDrawable(MESH,SHADER,CAMERA)

CAMERA:setEye(500000000,0,0)

state = lightSrc.mState

state:setPosition(5,3,2)

state.mAmbientColour.x = 99999
state.mAmbientColour.y = 99999
state.mAmbientColour.z = 99999

state.mScale.x = 0.2
state.mScale.y = 0.2
state.mScale.z = 0.2

DRAWMAN:addDrawable(lightSrc)

for i=1,1000 do
cubeptr = BasicDrawable(MESH,SHADER,CAMERA)

a = math.random()
b = math.random()
c = math.random()

posx = 100 * a - 50 + 500000000
posy = 100 * b - 50
posz = 100 * c - 50

cubeptr.mState:setPosition(posx,posy,posz)

dcol = cubeptr.mState.mDiffuseColour

dcol.x = a
dcol.y = b
dcol.z = c


DRAWMAN:addDrawable(cubeptr)
end