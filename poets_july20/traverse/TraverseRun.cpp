// SPDX-License-Identifier: BSD-2-Clause

#include "Traverse.h"

#include <HostLink.h>
#include <POLite.h>
#include <assert.h>
#include <sys/time.h>
#include <config.h>

#include "Mapping.h"

#define PRINT_RESULT 0

int main(int argc, char**argv)
{
	printf("Flags: ASYNC=%d, IGNORE_WEIGHTS=%d\n", ASYNC, IGNORE_WEIGHTS);
	
	if(argc != 2) {
		printf("Specify input file\n");
		exit(1);
	}

	struct timeval startAll, finishAll;
	gettimeofday(&startAll, NULL);

	// Read input
	// Input format: GitHub/ashurrafiev/ggraph
	FILE* in = fopen(argv[1], "rt");
	if(in==NULL) {
		fprintf(stderr, "Can't read file: %s\n", argv[1]);
		exit(1);
	}
	
	size_t totalNodes, totalEdges, maxFanout;
	assert(fscanf(in, "%zu %zu %zu", &totalNodes, &totalEdges, &maxFanout)==3);
	
	// create graph
	printf("Loading...\n");
	PGraph<SSSPDevice, SSSPState, int32_t, SSSPMessage> graph;
	for(size_t i = 0; i < totalNodes; i++) {
		PDeviceId id = graph.newDevice();
		assert(i == id);
	}
	
	printf("Nodes: %zu\nEdges: %zu\n", totalNodes, totalEdges);
	printf("Fanout: %zu\n", maxFanout);
	
	// read edge weights and create edges
	for(size_t i = 0; i < totalNodes; i++) {
		int32_t ncost; // ignored
		size_t edges;
		assert(fscanf(in, "%d %zu", &ncost, &edges)==2);
		for(size_t j=0; j<edges; j++) {
			size_t dst;
			int32_t w;
			assert(fscanf(in, "%zu %d", &dst, &w)==2);
			graph.addLabelledEdge(w, i, 0, dst);
			totalEdges--;
		}
	}
	
	fclose(in);
	assert(totalEdges==0);
	
	printf("Creating host link...\n");
	HostLink hostLink;

	printf("Mapping...\n");
	graph.mapVerticesToDRAM = true;
	graph.mapEdgesToDRAM = true;
	MAP(graph);

	int32_t rootNode = 0; // SSSP source id
	for(size_t i = 0; i < totalNodes; i++) {
		SSSPState* dev = &graph.devices[i]->state;
		dev->node = i;
		dev->isSource = (i==rootNode);
		dev->hops = 0;
		dev->dist = dev->isSource ? 0 : -1;
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

	int64_t sum = 0;
	int32_t max = 0;
	for(int32_t i = 0; i < graph.numDevices; i++) {
		PMessage<int32_t, SSSPMessage> msg;
		hostLink.recvMsg(&msg, sizeof(msg));
		if(i == 0) {
			// Stop timer
			gettimeofday(&finishCompute, NULL);
		}
		#if PRINT_RESULT
			printf("%d %d %d\n", msg.payload.node, msg.payload.hops, msg.payload.dist);
		#endif
		sum += msg.payload.dist;
		if(msg.payload.hops > max)
			max = msg.payload.hops;
	}
	
	printf("sum = %ld\nmax hops = %d\n", sum, max);

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
