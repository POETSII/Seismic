// SPDX-License-Identifier: BSD-2-Clause
#include "QSeismic.h"

#include <HostLink.h>
#include <POLite.h>
#include <assert.h>
#include <sys/time.h>
#include <config.h>

#include "Mapping.h"

int main(int argc, char**argv)
{
	if(argc != 2) {
		printf("Specify input file\n");
		exit(1);
	}

	struct timeval startAll, finishAll;
	gettimeofday(&startAll, NULL);

	// Read input
	FILE* in = fopen(argv[1], "rt");
	if(in==NULL) {
		fprintf(stderr, "Can't read file: %s\n", argv[1]);
		exit(1);
	}
	
	int32_t w, h, step, totalEdges;
	assert(fscanf(in, "%d %d %d", &w, &h, &step)==3);
	
	// create graph
	printf("Loading...\n");
	PGraph<QSeismicDevice, QSeismicState, None, QSeismicMessage> graph;
	for(uint32_t j = 0; j < h; j++) {
		for(uint32_t i = 0; i < w; i++) {
			PDeviceId id = graph.newDevice();
			assert(j*w+i == id);
		}
	}
	
	for(uint32_t j = 0; j < h; j++) {
		for(uint32_t i = 0; i < w; i++) {
			int32_t src = j*w+i;
			if(i>0) graph.addEdge(src, 0, src-1);
			if(i<w-1) graph.addEdge(src, 0, src+1);
			if(j>0) graph.addEdge(src, 0, src-w);
			if(j<h-1) graph.addEdge(src, 0, src+w);
		}
	}
	
	// read fanout template
	int32_t r, maxFanout, a;
	assert(fscanf(in, "%d %d", &r, &maxFanout)==2);
	if(r!=R) {
		printf("Expected radius %d, got %d\n", R, r);
		exit(1);
	}
	int32_t** e = (int32_t**) calloc(r+1, sizeof(int32_t*));
	for(int32_t ri=0; ri<=r; ri++) {
		e[ri] = (int32_t*) calloc(r+1, sizeof(int32_t));
		for(int32_t rj=0; rj<=r; rj++) {
			assert(fscanf(in, "%d", &e[ri][rj])==1);
		}
	}
	
	assert(fscanf(in, "%d", &totalEdges)==1);
	printf("Nodes: %d\nEdges: %d\n", w*h, totalEdges);

	printf("Creating host link...\n");
	HostLink hostLink;

	printf("Mapping...\n");
	//graph.mapVerticesToDRAM = true;
	//graph.mapEdgesToDRAM = true;
	MAP(graph);

	int32_t maxw = 0;
	// read edge weights and create edges
	int32_t rootNode = 2*w+w/2; // TODO: source id from arg
	for(int32_t sj=0; sj<h; sj++)
		for(int32_t si=0; si<w; si++) {
			PDeviceId s = sj*w+si;
			QSeismicState* dev = &graph.devices[s]->state;
			dev->x = si;
			dev->y = sj;
			dev->isSource = (s==rootNode);
			
			for(int32_t rj=-r; rj<=r; rj++)
				for(int32_t ri=-r; ri<=r; ri++) {
					int32_t i = ri+R;
					int32_t j = rj+R;
					int32_t index = makeIndex(i, j);
					
					if(e[abs(ri)][abs(rj)]) {
						int32_t di = si+ri;
						int32_t dj = sj+rj;
						if(di<0 || dj<0 || di>=w || dj>=h) {
							dev->w[index] = -1;
						}
						else {
							PDeviceId d = dj*w+di;
							
							int32_t x;
							assert(fscanf(in, "%d", &x)==1);
							if(x>maxw)
								maxw = x;
							dev->w[index] = x;
							totalEdges--;
							
							//printf("%d %d %.3f\n", s, d, x);
						}
					}
				}
		}
	printf("max w = %d\n", maxw);
	
	fclose(in);
	assert(totalEdges==0);

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

	uint64_t sum = 0;
	// Accumulate sum at each device
	for(int32_t i = 0; i < graph.numDevices; i++) {
		PMessage<None, QSeismicMessage> msg;
		hostLink.recvMsg(&msg, sizeof(msg));
		if(i == 0) {
			// Stop timer
			gettimeofday(&finishCompute, NULL);
		}
		//printf("%d %d %d\n", msg.payload.x, msg.payload.y, msg.payload.dist);
		sum += msg.payload.dist;
	}
	
	printf("sum = %lu\n", sum);

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
