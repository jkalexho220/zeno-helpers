float interpolatePerlin(float start = 0, float end = 1, float percentage = 0) {
	// return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
	return((end - start) * (3.0 - percentage * 2.0) * xsPow(percentage, 2) + start);
	//return((end - start) * percentage + start);
}

/*
Generate Perlin Noise graph and return an integer index of the graph. Use this integer
as input for the other PerlinNoise functions
*/
int generatePerlinNoise(int size = 50, int granularity = 5) {
	int db = aiPlanCreate("perlinNoise", 8);
	int meta = zNewArray(mInt, 4, "perlinMeta");
	zSetInt(meta, 0, db);
	zSetInt(meta, 1, granularity);
	zSetInt(meta, 2, zNewArray(mFloat, 4, "perlinInterpolation"));
	zSetInt(meta, 3, size);
	int dimension = 1 + size / granularity;
	for(i=0; < dimension) {
		aiPlanAddUserVariableVector(db,i,"perlin"+i,dimension);
		for(j=0; < dimension) {
			trQuestVarSetFromRand("temp", 0, 6.283185, false);
			aiPlanSetUserVariableVector(db,i,j,vectorSetFromAngle(trQuestVarGet("temp")));
		}
	}
	return(meta);
}

float getPerlinNoise(int meta = 0, float x = 0, float y = 0) {
	int db = zGetInt(meta, 0);
	int granularity = zGetInt(meta, 1);
	int interpolation = zGetInt(meta, 2);
	int baseX = (1 * x) / granularity;
	int baseY = (1 * y) / granularity;
	vector pos = xsVectorSet(x - baseX * granularity, 0, y - baseY * granularity);
	vector dir = vector(0,0,0);
	float total = 0;
	float interpolateX = xsVectorGetX(pos) / granularity;
	float interpolateY = xsVectorGetZ(pos) / granularity;
	for(i=0; <= 1) {
		for(j=0; <= 1) {
			dir = pos - xsVectorSet(1.0 * granularity * i, 0, 1.0 * granularity * j);
			zSetFloat(interpolation, 2 * i + j, dotProduct(dir, aiPlanGetUserVariableVector(db, baseX + i, baseY + j)));
		}
	}
	total = interpolatePerlin(interpolatePerlin(zGetFloat(interpolation, 0), zGetFloat(interpolation, 1), interpolateY),interpolatePerlin(zGetFloat(interpolation, 2), zGetFloat(interpolation, 3), interpolateY), interpolateX);
	return(total);
}

bool coordinatesInPerlin(int meta = 0, int x = 0, int y = 0, int padding = 0) {
	int size = zGetInt(meta, 3);
	return(x >= padding && y >= padding && x <= size - padding && y <= size - padding);
}

vector perlinNormalVector(int meta = 0, vector pos = vector(0,0,0), float radius = 1.0) {
	float perlinNorth = getPerlinNoise(meta, xsVectorGetX(pos) + radius, xsVectorGetZ(pos) + radius);
	float perlinEast = getPerlinNoise(meta, xsVectorGetX(pos) + radius, xsVectorGetZ(pos) - radius);
	float perlinSouth = getPerlinNoise(meta, xsVectorGetX(pos) - radius, xsVectorGetZ(pos) - radius);
	float perlinWest = getPerlinNoise(meta, xsVectorGetX(pos) - radius, xsVectorGetZ(pos) - radius);
	return(xsVectorNormalize(xsVectorSet(perlinNorth - perlinWest + perlinEast - perlinSouth, 0, perlinNorth - perlinEast + perlinWest - perlinSouth)));
}

/*
As though we were rolling a marble down a hill until it reaches below a certain height in the perlin noise.

Input: x and y coordinates of where the marble will drop in the perlin grid
Returns a position in vector coordinates.
*/
vector perlinRoll(int meta = 0, int x = 0, int y = 0, int stepSize = 1, float height = 0, int padding = 5, bool debug = false) {
	vector pos = xsVectorSet(x, 0, y);
	vector dir = xsVectorSet(stepSize, 0, 0);
	vector prev = vector(0,0,0);
	vector choice = dir;
	float best = getPerlinNoise(meta, x, y);
	float current = best + 1;
	bool found = false;
	vector temp = vector(0,0,0);
	for(k=0; < 99) {
		found = false;
		float before = best;
		for(i = -1; <= 1) {
			for(j = -1; <= 1) {
				dir = xsVectorSet(i * stepSize, 0, j * stepSize);
				if ((xsVectorGetX(dir) * xsVectorGetX(prev) + xsVectorGetZ(dir) * xsVectorGetZ(prev) < 0) || (i * i + j * j == 0)) {
					continue;
				} else {
					temp = pos + dir;
					if (coordinatesInPerlin(meta, 0 + xsVectorGetX(temp), 0 + xsVectorGetZ(temp), padding)) {
						current = getPerlinNoise(meta, 0 + xsVectorGetX(temp), 0 + xsVectorGetZ(temp));
						if (current < best) {
							best = current;
							found = true;
							choice = dir;
						}
					}
				}
			}
		}
		if ((best < height) || (found == false)) {
			// we reached a valley, or the threshold
			break;
		} else {
			pos = pos + choice;
			prev = choice;
			if (debug) {
				trArmyDispatch("1,0","Phoenix Egg",1,xsVectorGetX(pos)*2,0,xsVectorGetZ(pos)*2,0,true);
			}
		}
	}
	return(gridToVector(pos));
}