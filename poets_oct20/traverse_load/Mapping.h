// SPDX-License-Identifier: BSD-2-Clause
#ifndef _MAPPING_H_
#define _MAPPING_H_

//#define MAP(g) g.map()
#define MAP(g) bucketFillMap(&g)

#define DEV_PER_THREAD 0
#define PRINT_INFO 1

template <typename G> void bucketFillMap(G* graph) {
	printf("-- bucketFillMap\n");

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

	uint32_t totalThreads = numBoardsX*numBoardsY*TinselMailboxMeshXLen*TinselMailboxMeshYLen*(1<<TinselLogThreadsPerMailbox);
	#if DEV_PER_THREAD
		uint32_t devPerThread = DEV_PER_THREAD;
	#else
		uint32_t devPerThread = (graph->numDevices + totalThreads -1)/totalThreads; // round-up division
	#endif
	
	#if PRINT_INFO
	printf("Boards: %u x %u\nMesh: %u x %u\nThreads per mailbox: %u\nTotal threads: %u\nDev per threads: %u\nMax devices: %u\nGraph devices: %u\n",
		numBoardsX, numBoardsY, TinselMailboxMeshXLen, TinselMailboxMeshYLen, 1<<TinselLogThreadsPerMailbox,
		totalThreads, devPerThread, totalThreads*devPerThread, graph->numDevices);
	#endif
	assert(totalThreads*devPerThread>=graph->numDevices);
		
	graph->releaseAll();
    graph->allocateMapping();
	
	uint32_t remaining = graph->numDevices;
	uint32_t devId = 0;
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
						graph->numDevicesOnThread[threadId] = numDevs;
						if(numDevs>0) {
							graph->fromDeviceAddr[threadId] = (PDeviceId*) malloc(sizeof(PDeviceId) * numDevs);
							for (uint32_t devNum = 0; devNum < numDevs; devNum++)
								graph->fromDeviceAddr[threadId][devNum] = devId + devNum;

							// Populate toDeviceAddr mapping
							assert(numDevs < maxLocalDeviceId());
							for (uint32_t devNum = 0; devNum < numDevs; devNum++) {
								PDeviceAddr devAddr = makeDeviceAddr(threadId, devNum);
								graph->toDeviceAddr[devId + devNum] = devAddr;
							}
							remaining -= numDevs;
							devId += numDevs;
						}
					}
				}
			}
		}
	}

	graph->allocatePartitions();
	graph->initialisePartitions();
}

#endif
