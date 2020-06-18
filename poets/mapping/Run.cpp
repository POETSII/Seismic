// SPDX-License-Identifier: BSD-2-Clause

#include "SSSP.h"

#include <HostLink.h>
#include <POLite.h>
#include <assert.h>
#include <sys/time.h>
#include <config.h>

#define PRINT_RESULT 0
#define POLITE_MAPPING 0
#define DEV_PER_THREAD 0

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
	
	char sbuf[16];
	assert(fscanf(in, "%s", sbuf)==1);
	assert(strcmp("T3.2", sbuf)==0);
	
	int32_t w, l, h, step;
	assert(fscanf(in, "%d %d %d %d", &w, &l, &h, &step)==4);
	
	// create graph
	printf("Loading...\n");
	int64_t totalNodes, totalEdges;
	PGraph<SSSPDevice, SSSPState, int32_t, SSSPMessage> graph;
	totalNodes = 0;
	for(uint32_t k = 0; k < h; k++) {
		for(uint32_t j = 0; j < l; j++) {
			for(uint32_t i = 0; i < w; i++) {
				PDeviceId id = graph.newDevice();
				assert((k*l+j)*w+i == id);
				totalNodes++;
			}
		}
	}
	
	// read fanout template
	int32_t r, maxFanout, a;
	assert(fscanf(in, "%d %d", &r, &maxFanout)==2);
	int32_t* e = (int32_t*) calloc((r+1)*(r+1)*(r+1), sizeof(int32_t));
	for(int32_t rk=0; rk<=r; rk++) {
		for(int32_t rj=0; rj<=r; rj++) {
			for(int32_t ri=0; ri<=r; ri++) {
				assert(fscanf(in, "%d", &e[(rk*(r+1)+rj)*(r+1)+ri])==1);
			}
		}
	}
	
	assert(fscanf(in, "%s", sbuf)==1);
	assert(strcmp("BEGIN", sbuf)==0);
	
	// read edge weights and create edges
	totalEdges = 0;
	for(int32_t sk=0; sk<h; sk++)
		for(int32_t sj=0; sj<l; sj++) {
			for(int32_t si=0; si<w; si++) {
				PDeviceId s = (sk*l+sj)*w+si;
				for(int32_t rk=-r; rk<=r; rk++)
					for(int32_t rj=-r; rj<=r; rj++)
						for(int32_t ri=-r; ri<=r; ri++) {
							if(e[(abs(rk)*(r+1)+abs(rj))*(r+1)+abs(ri)]) {
								int32_t di = si+ri;
								int32_t dj = sj+rj;
								int32_t dk = sk+rk;
								if(di<0 || dj<0 || dk<0)
									continue;
								int32_t d = (dk*l+dj)*w+di;
								if(s<d) {
									int32_t x;
									assert(fscanf(in, "%d", &x)==1);
									// printf("%d/%d %d/%d %d/%d R %d %d %d \n", si, w, sj, l, sk, h, ri, rj, rk);
									if(di<w && dj<l && dk<h) {
										graph.addLabelledEdge((int16_t)x, s, 0, d);
										totalEdges++;
									}
								}
							}
						}
			}
		}

	assert(fscanf(in, "%s", sbuf)==1);
	assert(strcmp("END", sbuf)==0);
		
	fclose(in);
	printf("%ld nodes, %ld edges\n", totalNodes, totalEdges);
	
	char* str = getenv("HOSTLINK_BOXES_X");
    uint32_t meshLenX = str ? atoi(str) : 1;
    meshLenX = meshLenX * TinselMeshXLenWithinBox;
    str = getenv("HOSTLINK_BOXES_Y");
    uint32_t meshLenY = str ? atoi(str) : 1;
    meshLenY = meshLenY * TinselMeshYLenWithinBox;
	
	str = getenv("POLITE_BOARDS_X");
    uint32_t  numBoardsX = str ? atoi(str) : meshLenX;
    str = getenv("POLITE_BOARDS_Y");
    uint32_t  numBoardsY = str ? atoi(str) : meshLenY;
	
	//printf("Done test\n");
	//return 0;

	printf("Creating host link...\n");
	HostLink hostLink;

#if POLITE_MAPPING
	printf("Mapping... (POLite)\n");
	//graph.mapVerticesToDRAM = true;
	//graph.mapEdgesToDRAM = true;
	graph.map();
#else
	printf("Mapping... (custom)\n");
	graph.mapVerticesToDRAM = true;
	graph.mapEdgesToDRAM = true;

	graph.releaseAll();
    graph.allocateMapping();
	
	uint32_t remaining = totalNodes;
	uint32_t devId = 0;
	#if DEV_PER_THREAD
		uint32_t devPerThread = DEV_PER_THREAD;
	#else
		uint32_t totalThreads = numBoardsX*numBoardsY*TinselMailboxMeshXLen*TinselMailboxMeshYLen*(1<<TinselLogThreadsPerMailbox);
		uint32_t devPerThread = (totalNodes + totalThreads -1)/totalThreads; // round-up division
		printf("Dev per thread: %u\n", devPerThread);
	#endif
	for (uint32_t boardY = 0; boardY < numBoardsY; boardY++) {
		for (uint32_t boardX = 0; boardX < numBoardsX; boardX++) {
			for (uint32_t boxX = 0; boxX < TinselMailboxMeshXLen; boxX++) {
				for (uint32_t boxY = 0; boxY < TinselMailboxMeshYLen; boxY++) {
					uint32_t numThreads = 1<<TinselLogThreadsPerMailbox;
					for (uint32_t threadNum = 0; threadNum < numThreads; threadNum++) {
						if(!remaining)
							break;
						
						// Determine tinsel thread id
						uint32_t threadId = boardY;
						threadId = (threadId << TinselMeshXBits) | boardX;
						threadId = (threadId << TinselMailboxMeshYBits) | boxY;
						threadId = (threadId << TinselMailboxMeshXBits) | boxX;
						threadId = (threadId << (TinselLogCoresPerMailbox +
									TinselLogThreadsPerCore)) | threadNum;

						// Populate fromDeviceAddr mapping
						uint32_t numDevs = (remaining<devPerThread) ? remaining : devPerThread;
						graph.numDevicesOnThread[threadId] = numDevs;
						if(numDevs>0) {
							graph.fromDeviceAddr[threadId] = (PDeviceId*) malloc(sizeof(PDeviceId) * numDevs);
							for (uint32_t devNum = 0; devNum < numDevs; devNum++)
								graph.fromDeviceAddr[threadId][devNum] = devId + devNum;

							// Populate toDeviceAddr mapping
							assert(numDevs < maxLocalDeviceId());
							for (uint32_t devNum = 0; devNum < numDevs; devNum++) {
								PDeviceAddr devAddr = makeDeviceAddr(threadId, devNum);
								graph.toDeviceAddr[devId + devNum] = devAddr;
							}
							remaining -= numDevs;
							devId += numDevs;
						}
					}
				}
			}
		}
	}

	graph.allocatePartitions();
	graph.initialisePartitions();
#endif

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

	uint64_t sum = 0;
	// Accumulate sum at each device
	for(int32_t i = 0; i < graph.numDevices; i++) {
		PMessage<int32_t, SSSPMessage> msg;
		hostLink.recvMsg(&msg, sizeof(msg));
		if(i == 0) {
			// Stop timer
			gettimeofday(&finishCompute, NULL);
		}
		#if PRINT_RESULT
		printf("%d %d %d\n", msg.payload.node, msg.payload.from, msg.payload.dist);
		#endif
		sum += msg.payload.dist;
	}
	
	printf("sum = %lu\n", sum);

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
