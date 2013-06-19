/* This code has been adapted from an implementation by Eliot Eshelman.
https://code.google.com/p/battlestar-tux/source/browse/procedural/simplexnoise.cpp
http://www.6by9.net/simplex-noise-for-c-and-python/
*/

//Something something GPL

float dot20(uint simpData, const float x, const float y);
float dot21(uint simpData, const float x, const float y);
float dot22(uint simpData, const float x, const float y);

StructuredBuffer<uint> simplexBuffer : register(t0);

float noise2D( float x, float y) {
	
	// Noise contributions from the three corners
    float n0, n1, n2;

    // Skew the input space to determine which simplex cell we're in
    const float F2 = 0.5f * (sqrt(3.0f) - 1.0f);
    // Hairy factor for 2D
    float s = (x + y) * F2;
    int i = floor( x + s );
    int j = floor( y + s );

    const float G2 = (3.0f - sqrt(3.0f)) / 6.0f;
    float t = (i + j) * G2;
    // Unskew the cell origin back to (x,y) space
    float X0 = i-t;
    float Y0 = j-t;
    // The x,y distances from the cell origin
    float x0 = x-X0;
    float y0 = y-Y0;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
	
    // Offsets for second (middle) corner of simplex in (i,j) coords
	int i1 = x0>y0; // lower triangle, XY order: (0,0)->(1,0)->(1,1)
	int j1 = !(i1); // upper triangle, YX order: (0,0)->(0,1)->(1,1)

    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6
    float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
    float y2 = y0 - 1.0 + 2.0 * G2;

    // Work out the hashed gradient indices of the three simplex corners
    uint ii = i & 255;
    uint jj = j & 255;
	
	uint simpData = simplexBuffer[256*ii+jj];
	simpData = simpData >> (16*j1);

    // Calculate the contribution from the three corners
    float t0 = 0.5 - x0*x0-y0*y0;
	t0 = (t0<0) ? 0 : t0;
	t0 *= t0;
	n0 = t0 * t0 * dot20(simpData,x0,y0);

    float t1 = 0.5 - x1*x1-y1*y1;
	t1 = (t1<0) ? 0 : t1;
	t1 *= t1;
	n1 = t1 * t1 * dot21(simpData, x1, y1);

    float t2 = 0.5 - x2*x2-y2*y2;
	t2 = (t2<0) ? 0 : t2;
    t2 *= t2;
	n2 = t2 * t2 * dot22(simpData, x2, y2);

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.0f * (n0 + n1 + n2);
}

float dot20(uint simpData, const float x, const float y) {
	int gx = (int)(simpData & 3) - 1;
	int gy = (int)((simpData & 12) >> 2) - 1;
	return gx*x+gy*y;
}

float dot21(uint simpData, const float x, const float y) {
	int gx = (int)((simpData & 48) >> 4) - 1;
	int gy = (int)((simpData & 192) >> 6) - 1;
	return gx*x+gy*y;
}

float dot22(uint simpData, const float x, const float y) {
	int gx = (int)((simpData & 768) >> 8) - 1;
	int gy = (int)((simpData & 3072) >> 10) - 1;
	return gx*x+gy*y;
}