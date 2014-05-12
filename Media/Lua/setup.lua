--Globals
RMP = 0
CAMERA = 0
SHADERMAN = 0
MESHMAN = 0
MESH = 0
DRAWMAN = 0
SET = 0
GENERATOR = 0
-- Set up console printer
function setRMP(pRMP)
RMP = pRMP
end

function print(X)
RMP:luaLog(X)
end

function run(X)
RMP:runScript(X)
end

function setCamera(pCam)
CAMERA = pCam
end

function setShaderMan(pShaderMan)
SHADERMAN = pShaderMan
end

function setMeshMan(pMeshMan)
MESHMAN = pMeshMan
end

function setDrawMan(pDrawMan)
DRAWMAN = pDrawMan
end

function setRendererSettings(pRSet)
SET = pRSet
end

function setGenerator(pGen)
GENERATOR = pGen
end

function reset()
DRAWMAN:reset()
GENERATOR:reset()
end

function move(x,y,z)
CAMERA:setEye(x,y,z)
end

function smove(x,y,z,speed)
CAMERA:smoothMove(x,y,z,speed)
end

function cmove(x,y,z)
CAMERA:constMove(x,y,z)
end

function look(x,y)
CAMERA:setLook(x,y)
end

function slook(x,y,speed)
CAMERA:smoothLook(x,y,speed)
end

function clook(x,y)
CAMERA:constLook(x,y)
end

function sched(x)
SET.schedTime = x
end

function step()
SET.schedStep = 1
end

function speed(x)
SET.cameraspeed = x
end

function wf(x)
SET.wireframe = x
end

function tess(x)
SET.tessAmount = x
end

function initload()
GENERATOR:setInitialLoad()
end

function runcube()
run("cube.lua")
end

function runwater()
run("water.lua")
end

function runmount()
run("mountains.lua")
end

function runearth()
run("earth.lua")
end

lastWorld = 0;

function run2(x)
if lastWorld ~= x then run(x) end
lastWorld = x;
end

cue = 0;

