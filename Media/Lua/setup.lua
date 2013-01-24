--Globals
RMP = 0
CAMERA = 0
SHADER = 0
MESH = 0
DRAWMAN = 0
-- Set up console printer
function setRMP(pRMP)
RMP = pRMP
end

function print(X)
RMP:luaLog(X)
end

function runScript(X)
RMP:runScript(X)
end

function setCamera(pCam)
CAMERA = pCam
end

function setShader(pShader)
SHADER = pShader
end

function setMesh(pMesh)
MESH = pMesh
end

function setDrawMan(pDrawMan)
DRAWMAN = pDrawMan
end