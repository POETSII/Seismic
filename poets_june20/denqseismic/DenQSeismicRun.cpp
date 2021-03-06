// SPDX-License-Identifier: BSD-2-Clause
#include "DenQSeismic.h"

#include <HostLink.h>
#include <POLite.h>
#include <assert.h>
#include <sys/time.h>
#include <config.h>

#include "Mapping.h"

int32_t readIntBE(FILE* in) {
	uint8_t b;
	int32_t x = 0;
	for(int32_t d=0; d<4; d++) {
		assert(fread(&b, 1, 1, in)==1);
		x = (x<<8) | b;
	}
	return x;
}

int main(int argc, char**argv)
{
	if(argc != 2) {
		printf("Specify input file\n");
		exit(1);
	}

	struct timeval startAll, finishAll;
	gettimeofday(&startAll, NULL);

	// Read input
	printf("Loading density map...\n");
	FILE* in = fopen(argv[1], "rt");
	if(in==NULL) {
		fprintf(stderr, "Can't read file: %s\n", argv[1]);
		exit(1);
	}
	
	int32_t w, h;
	w = readIntBE(in);
	h = readIntBE(in);
	uint8_t* d = (uint8_t*) calloc(w*h, 1);
	for(int32_t y=0; y<h; y++) {
		for(int32_t x=0; x<w; x++) {
			assert(fread(&d[y*w+x], 1, 1, in)==1);
		}
	}
	fclose(in);
	
	// create graph
	printf("Creating devices...\n");
	PGraph<DenQSeismicDevice, DenQSeismicState, None, DenQSeismicMessage> graph;
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
	
	printf("Nodes: %d\n", w*h);

	printf("Creating host link...\n");
	HostLink hostLink;

	printf("Mapping...\n");
	//graph.mapVerticesToDRAM = true;
	//graph.mapEdgesToDRAM = true;
	MAP(graph);

	// read edge weights and create edges
	int32_t rootNode = 2*w+w/2; // TODO: source id from arg
	for(int32_t sj=0; sj<h; sj++)
		for(int32_t si=0; si<w; si++) {
			PDeviceId s = sj*w+si;
			DenQSeismicState* dev = &graph.devices[s]->state;
			dev->x = si;
			dev->y = sj;
			dev->isSource = (s==rootNode);
			
			for(int32_t rj=-R; rj<=R; rj++)
				for(int32_t ri=-R; ri<=R; ri++) {
					int32_t i = ri+R;
					int32_t j = rj+R;
					int32_t index = makeIndex(i, j);
					
					int32_t di = si+ri;
					if(di<0) di = 0;
					if(di>=w) di = w-1;
					int32_t dj = sj+rj;
					if(dj<0) dj = 0;
					if(dj>=h) dj = h-1;
							
					dev->changed[index] = d[dj*w+di];
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

	uint64_t sum = 0;
	// Accumulate sum at each device
	for(int32_t i = 0; i < graph.numDevices; i++) {
		PMessage<None, DenQSeismicMessage> msg;
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
