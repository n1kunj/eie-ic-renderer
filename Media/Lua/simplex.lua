reset()

numNoises = 3
numBiomes = 1


scales = {100000,50000,25000}
		
coeffs = {
10000,5000,2500
};

cols = {
1,1,1,1,
};

spec = {
20,0,
};

distDr = DistantDrawable(CAMERA,SHADERMAN,MESHMAN,GENERATOR,6,15,42.5);
GENERATOR:setCityHandicap(0);
GENERATOR:setNoiseBiomeCount(numNoises,numBiomes);

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