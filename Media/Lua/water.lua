reset()

numNoises = 12
numBiomes = 5


scales = {150000, 30000, 12800,	640,
		320, 160, 80, 40,
		20, 10, 5, 2.5}
		
coeffs = {
50,-100,100,-10,10,-1,1,-1,1,-1,0.5,0.5,
50,-100,100,-10,10,-1,1,-1,1,-1,0.5,0.5,
50,-100,100,-10,10,-1,1,-1,1,-1,0.5,0.5,
50,-100,100,-10,10,-1,1,-1,1,-1,0.5,0.5,
50,-100,100,-10,10,-1,1,-1,1,-1,0.5,0.5,
};

cols = {
0,0,0.5,1,
0,0,0.4,1,
0,0,0.3,1,
0,0,0.2,1,
0,0,0.1,1,
};

spec = {
20,1,
20,1,
20,1,
20,1,
20,1,
};

GENERATOR:setCityHandicap(0);
GENERATOR:setNoiseBiomeCount(numNoises,numBiomes);
distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,6,15,42.5*2);

for i=1,numNoises do
GENERATOR:setScalesData(i-1,scales[i]);
end

for i=1,numNoises*numBiomes do
GENERATOR:setCoeffsData(i-1,coeffs[i]);
end	

for i=0,numBiomes-1 do
GENERATOR:setColourCityData(i,cols[i*4+1],cols[i*4+2],cols[i*4+3],cols[i*4+4]);
end

for i=0,numBiomes-1 do
GENERATOR:setSpecPowData(i,spec[i*2+1],spec[i*2+2]);
end

DRAWMAN:addDrawable(distDr)