#include <string.h>
#include <unistd.h>
#define PATH_MAX 4096

#include "Net.h"

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
	int32_t step = DEFAULT_STEP;
	int32_t radius = DEFAULT_RADIUS;
	int32_t allEdges = 0;
	
	int opt;
	while((opt = getopt(argc, argv, "i:o:b:s:g:r:a:")) != -1) {
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
				step = atoi(optarg);
				break;
			case 'r':
				radius = atoi(optarg);
				break;
			case 'a':
				allEdges = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s -i input [-b bias] [-s scale] [-g step] [-r radius] [-o output] [-a fanout_algorithm]\n", argv[0]);
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
	SourceData src;
	readSourceData(in, &src, bias, scale);
	fclose(in);
	
	Fanout fanout;
	if(allEdges==1)
		fullFanout(&fanout, radius);
	else if(allEdges==2)
		bresenhamFanout(&fanout, radius);
	else
		forwardStar(&fanout, radius);
	
	FILE* out = fopen(outPath, "wt");
	if(out==NULL) {
		fprintf(stderr, "Can't write output to %s\n", outPath);
		exit(1);
	}
	writeNet(out, &src, step, &fanout);
	fclose(out);

	return 0;
}
