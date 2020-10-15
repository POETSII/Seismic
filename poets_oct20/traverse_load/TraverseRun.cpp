// SPDX-License-Identifier: BSD-2-Clause

#include "Traverse.h"

#include <HostLink.h>
#include <POLite.h>
#include <assert.h>
#include <sys/time.h>
#include <config.h>

#include "LoadGraph.h"
#include "Mapping.h"

#define PRINT_RESULT 0

int main(int argc, char**argv)
{
	printf("Flags: ASYNC=%d, IGNORE_WEIGHTS=%d, FLAT_WEIGHTS=%d\n", ASYNC, IGNORE_WEIGHTS, FLAT_WEIGHTS);
	
	if(argc != 2) {
		printf("Specify input file\n");
		exit(1);
	}

	struct timeval startAll, finishAll;
	gettimeofday(&startAll, NULL);

	LoadGraph g;
	g.load(argv[1]);
	
	// create graph
	printf("Creating ...\n");
	PGraph<SSSPDevice, SSSPState, int32_t, SSSPMessage> graph;
	for(size_t i = 0; i < g.numNodes; i++) {
		PDeviceId id = graph.newDevice();
		assert(i == id);
	}
	
	// read edge weights and create edges
	for(size_t i = 0; i < g.numNodes; i++) {
		LoadNode &node = g.nodes[i];
		for(size_t j=0; j<node.edges.size(); j++) {
			LoadEdge &edge = node.edges[j];
			graph.addLabelledEdge(edge.cost, i, 0, edge.dst);
		}
	}
	
	printf("rootNode = %d\n", g.rootNode);
	
	printf("Creating host link...\n");
	HostLink hostLink;

	printf("Mapping...\n");
	graph.mapVerticesToDRAM = true;
	graph.mapEdgesToDRAM = true;
	MAP(graph);

	for(size_t i = 0; i < g.numNodes; i++) {
		SSSPState* dev = &graph.devices[i]->state;
		dev->node = i;
		dev->isSource = (i==g.rootNode);
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

	#if PRINT_RESULT
	FILE* out = fopen("result.txt", "wt");
	if(out==NULL) {
		fprintf(stderr, "Can't write output file\n");
		exit(EXIT_FAILURE);
	}
	fprintf(out, "%s\n%d\n", argv[1], totalNodes);
	#endif
	
	int64_t sum = 0;
	int32_t max = 0;
	for(int32_t i = 0; i < graph.numDevices; i++) {
		PMessage<int32_t, SSSPMessage> msg;
		hostLink.recvMsg(&msg, sizeof(msg));
		if(i == 0) {
			// Stop timer
			gettimeofday(&finishCompute, NULL);
		}
		if(g.ignoreNodes[msg.payload.node])
			continue;
		#if PRINT_RESULT
			#if COUNT_VISITS
			fprintf(out, "%d %d %d %d\n", msg.payload.node, msg.payload.hops, msg.payload.dist, msg.payload.visits);
			#else
			fprintf(out, "%d %d %d\n", msg.payload.node, msg.payload.hops, msg.payload.dist);
			#endif
		#endif
		sum += msg.payload.dist;
		if(msg.payload.hops > max)
			max = msg.payload.hops;
	}
	
	printf("sum = %ld\nmax hops = %d\n", sum, max);
	#if PRINT_RESULT
	fclose(out);
	#endif

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
