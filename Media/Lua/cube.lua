function runcube()
run("cube.lua")
end

reset()

lightSrc = BasicDrawable(MESH,SHADER,CAMERA)

state = lightSrc.mState

state.mPosition.x = 5
state.mPosition.y = 3
state.mPosition.z = 2

state.mAmbientColour.x = 99999
state.mAmbientColour.y = 99999
state.mAmbientColour.z = 99999

state.mScale.x = 0.2
state.mScale.y = 0.2
state.mScale.z = 0.2

DRAWMAN:addDrawable(lightSrc)

for i=1,10000 do
cubeptr = BasicDrawable(MESH,SHADER,CAMERA)

pos = cubeptr.mState.mPosition

dcol = cubeptr.mState.mDiffuseColour

a = math.random()
b = math.random()
c = math.random()

pos.x = 100 * a - 50
pos.y = 100 * b - 50
pos.z = 100 * c - 50

dcol.x = a
dcol.y = b
dcol.z = c


DRAWMAN:addDrawable(cubeptr)
end