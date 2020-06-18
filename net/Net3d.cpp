#include <string.h>
#include <unistd.h>
#define PATH_MAX 4096

#include "Net3d.h"

#define DEFAULT_BIAS 0.25
#define DEFAULT_SCALE 1
#define DEFAULT_STEP 4
#define DEFAULT_RADIUS 1

int main(int argc, char *argv[])
{
	char inPath[PATH_MAX];
	bool inSet = false;
	char outPath[PATH_MAX] = "out.txt";
	float bias = DEFAULT_BIAS;
	float scale = DEFAULT_SCALE;
	int32_t stepx = DEFAULT_STEP;
	int32_t stepy = DEFAULT_STEP;
	int32_t stepz = DEFAULT_STEP;
	int32_t radius = DEFAULT_RADIUS;
	bool spherical = false;
	
	int opt;
	while((opt = getopt(argc, argv, "i:o:b:s:g:r:c")) != -1) {
		switch (opt) {
			case 'i':
				strncpy(inPath, optarg, PATH_MAX);
				inSet = true;
				break;
			case 'o':
				strncpy(outPath, optarg, PATH_MAX);
				break;
			case 'b':
				bias = atof(optarg);
				break;
			case 's':
				scale = atof(optarg);
				break;
			case 'g':
				stepx = optarg[0]-0x30;
				if(stepx<1) stepx = 1;
				stepy = optarg[1]-0x30;
				if(stepy<1) stepy = 1;
				stepz = optarg[2]-0x30;
				if(stepz<1) stepz = 1;
				break;
			case 'r':
				radius = atoi(optarg);
				break;
			case 'c':
				spherical = true;
				break;
			default:
				fprintf(stderr, "Usage: %s -i input [-b bias] [-s scale] [-g xyz] [-r radius] [-c] [-o output]\n", argv[0]);
				exit(1);
				break;
		}
	}
	if(!inSet) {
		fprintf(stderr, "Input file?\n");
		exit(1);
	}
	
	FILE* in = fopen(inPath, "rb");
	if(in==NULL) {
		fprintf(stderr, "Can't read input from %s\n", inPath);
		exit(1);
	}
	SourceData3D src;
	readSourceData(in, &src, bias, scale);
	fclose(in);
	printf("Source data loaded\n");
	
	Fanout3D fanout;
	//forwardStar3D(&fanout, radius, spherical);
	bresenhamFanout3D(&fanout, radius);
	printf("Fanout calculated\n");
	
	FILE* out = fopen(outPath, "wt");
	if(out==NULL) {
		fprintf(stderr, "Can't write output to %s\n", outPath);
		exit(1);
	}
	writeNet(out, &src, stepx, stepy, stepz, &fanout);
	fclose(out);

	return 0;
}
