// SPDX-License-Identifier: BSD-2-Clause
#include "ASP.h"

#include <HostLink.h>
#include <POLite.h>
#include <EdgeList.h>
#include <assert.h>
#include <sys/time.h>
#include <config.h>

#define ROOT_ID 0

int main(int argc, char**argv)
{
	if (argc != 2) {
		printf("Specify edges file\n");
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

	uint32_t numNodes;
	assert(fscanf(in, "%d", &numNodes)==1);
	
	// create graph
	printf("Loading...\n");
	PGraph<ASPDevice, ASPState, EDGE_TYPE, ASPMessage> graph;
	for(uint32_t i = 0; i < numNodes; i++) {
		PDeviceId id = graph.newDevice();
		assert(i == id);
	}
	// read edge weights and create edges
	uint32_t numEdges, dest;
	float w;
	for(uint32_t i=0; i<numNodes; i++) {
		assert(fscanf(in, "%d", &numEdges)==1);
		for(uint32_t j=0; j<numEdges; j++) {
			assert(fscanf(in, "%d %f", &dest, &w)==2);
			assert(i!=dest);
			
#if FIXED_WEIGHTS
			graph.addLabelledEdge(1, i, 0, dest);
#else
			graph.addLabelledEdge((EDGE_TYPE)w, i, 0, dest);
#endif

			//graph.addEdge(i, 0, dest);
		}
	}

	fclose(in);
	// Connection to tinsel machine
	printf("Creating host link...\n");
	HostLink hostLink;

	// Prepare mapping from graph to hardware
	printf("Mapping...\n");
	graph.map();

	// Create nodes in POETS graph
	for(uint32_t i = 0; i < numNodes; i++) {
		ASPState* dev = &graph.devices[i]->state;
		dev->nodeId = i;
		dev->root = (i==ROOT_ID) ? 1 : 0;
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
	//politeSaveStats(&hostLink, "stats.txt");

	// Sum of all shortest paths
	float sum = 0;

	// Accumulate sum at each device
	for(uint32_t i = 0; i < graph.numDevices; i++) {
		PMessage<EDGE_TYPE, ASPMessage> msg;
		hostLink.recvMsg(&msg, sizeof(msg));
		if(i == 0) {
			// Stop timer
			gettimeofday(&finishCompute, NULL);
		}
		float dist = msg.payload.dist;
		printf("%d: %f\n", msg.payload.nodeId, dist);
		sum += dist;
	}

	// Emit sum
	printf("Sum = %f\n", sum);

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
