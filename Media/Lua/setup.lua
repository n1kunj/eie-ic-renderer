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
end

function move(x,y,z)
CAMERA:setEye(x,y,z)
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