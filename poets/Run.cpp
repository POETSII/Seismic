// SPDX-License-Identifier: BSD-2-Clause
#include "SSSP.h"

#include <HostLink.h>
#include <POLite.h>
#include <assert.h>
#include <sys/time.h>
#include <config.h>

int main(int argc, char**argv)
{
	if(argc != 2) {
		printf("Specify input file\n");
		exit(EXIT_FAILURE);
	}

	struct timeval startAll, finishAll;
	gettimeofday(&startAll, NULL);

	// Read input
	FILE* in = fopen(argv[1], "rt");
	if(in==NULL) {
		fprintf(stderr, "Can't read file: %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	
	int32_t w, h, step, totalEdges;
	fscanf(in, "%d %d %d", &w, &h, &step);
	
	// create graph
	printf("Loading...\n");
	PGraph<SSSPDevice, SSSPState, int32_t, SSSPMessage> graph;
	for(uint32_t j = 0; j < h; j++) {
		for(uint32_t i = 0; i < w; i++) {
			PDeviceId id = graph.newDevice();
			assert(j*w+i == id);
		}
	}
	
	// read fanout template
	int32_t r, maxFanout, a;
	fscanf(in, "%d %d", &r, &maxFanout);
	int32_t** e = (int32_t**) calloc(r+1, sizeof(int32_t*));
	for(int32_t ri=0; ri<=r; ri++) {
		e[ri] = (int32_t*) calloc(r+1, sizeof(int32_t));
		for(int32_t rj=0; rj<=r; rj++) {
			fscanf(in, "%d", &e[ri][rj]);
		}
	}
	
	fscanf(in, "%d", &totalEdges);
	printf("Nodes: %d\nEdges: %d\n", w*h, totalEdges);
	
	// read edge weights and create edges
	for(int32_t sj=0; sj<h; sj++)
		for(int32_t si=0; si<w; si++) {
			PDeviceId s = sj*w+si;
			for(int32_t rj=-r; rj<=r; rj++)
				for(int32_t ri=-r; ri<=r; ri++) {
					if(e[abs(ri)][abs(rj)]) {
						int32_t di = si+ri;
						int32_t dj = sj+rj;
						if(di<0 || dj<0 || di>=w || dj>=h)
							continue;
						PDeviceId d = dj*w+di;
						
						float x = 1.0;
						fscanf(in, "%f", &x);
						graph.addLabelledEdge((int32_t)(x*1000), s, 0, d); // FIXME: Causes error for r>=3: corrupted size vs. prev_size
						totalEdges--;
						
						//printf("%d %d %.3f\n", s, d, x);
					}
				}
		}
		
	fclose(in);
	assert(totalEdges==0);
	
	return 1; // Temporary block. The error happens (if it happens) before that.

	printf("Creating host link...\n");
	HostLink hostLink;

	printf("Mapping...\n");
	graph.map();

	int32_t rootNode = 2*w+w/2; // TODO: source id from arg
	for(int32_t j = 0; j < h; j++) {
		for(int32_t i = 0; i < w; i++) {
			int32_t node = j*w+i;
			SSSPState* dev = &graph.devices[node]->state;
			dev->node = node;
			dev->isSource = (node==rootNode);
			dev->parent = -1;
			dev->dist = dev->isSource ? 0 : -1;
		}
	}

	// Write graph down to tinsel machine via HostLink
	graph.write(&hostLink);

	// Load code and trigger execution
	hostLink.boot("code.v", "data.v");
	hostLink.go();

	// Timer
	printf("\nStarted\n");
	struct timeval startCompute, finishCompute;
	gettimeofday(&startCompute, NULL);

	// Consume performance stats
	politeSaveStats(&hostLink, "stats.txt");

	// Write output
	/*FILE* out = fopen("out.txt", "wt");
	if(out==NULL) {
		fprintf(stderr, "Can't write output file\n");
		exit(EXIT_FAILURE);
	}
	fprintf(out, "%d %d %d\n", w, h, step);*/

	// Accumulate sum at each device
	for(int32_t i = 0; i < graph.numDevices; i++) {
		PMessage<int32_t, SSSPMessage> msg;
		hostLink.recvMsg(&msg, sizeof(msg));
		if(i == 0) {
			// Stop timer
			gettimeofday(&finishCompute, NULL);
		}
		float x = (float)msg.payload.dist / 1000.0;
		printf("%d %d %f\n", msg.payload.node, msg.payload.from, x);
	}

	//fclose(out);

	// Display time
	struct timeval diff;
	double duration;
	timersub(&finishCompute, &startCompute, &diff);
	duration = (double) diff.tv_sec + (double) diff.tv_usec / 1000000.0;
	printf("Time (compute) = %lf\n", duration);
	gettimeofday(&finishAll, NULL);
	timersub(&finishAll, &startAll, &diff);
	duration = (double) diff.tv_sec + (double) diff.tv_usec / 1000000.0;
	printf("Time (all) = %lf\n", duration);

	return 0;
}
