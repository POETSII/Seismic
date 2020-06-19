// SPDX-License-Identifier: BSD-2-Clause
#include "DenQSeismic3D.h"

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
	int32_t w = 0;
	int32_t l = 0;
	int32_t h = 0;
	if(argc==5) {
		w = atoi(argv[1]);
		l = atoi(argv[2]);
		h = atoi(argv[3]);
	}
	if(w<1 || l<1 || h<1) {
		printf("Usage: ./run w l h input\n");
		exit(1);
	}

	struct timeval startAll, finishAll;
	gettimeofday(&startAll, NULL);

	// Read input
	printf("Loading density map...\n");
	FILE* in = fopen(argv[4], "rt");
	if(in==NULL) {
		fprintf(stderr, "Can't read file: %s\n", argv[1]);
		exit(1);
	}
	
	int32_t sw, sl, sh;
	sw = readIntBE(in);
	sl = readIntBE(in);
	sh = readIntBE(in);
	uint8_t* d = (uint8_t*) calloc(sw*sl*sh, 1);
	for(int32_t x=0; x<sw; x++) {
		for(int32_t y=0; y<sl; y++) {
			for(int32_t z=0; z<sh; z++) {
				assert(fread(&d[(z*sl+y)*sw+x], 1, 1, in)==1);
			}
		}
	}
	fclose(in);
	
	// create graph
	printf("Creating devices...\n");
	PGraph<DenQSeismic3DDevice, DenQSeismic3DState, None, DenQSeismic3DMessage> graph;
	for(int32_t k=0; k<h; k++) {
		for(int32_t j=0; j<l; j++) {
			for(int32_t i=0; i<w; i++) {
				PDeviceId id = graph.newDevice();
				assert((k*l+j)*w+i == id);
			}
		}
	}
	
	for(int32_t k=0; k<h; k++) {
		for(int32_t j=0; j<l; j++) {
			for(int32_t i=0; i<w; i++) {
				int32_t src = (k*l+j)*w+i;
				if(i>0) graph.addEdge(src, 0, src-1);
				if(i<w-1) graph.addEdge(src, 0, src+1);
				if(j>0) graph.addEdge(src, 0, src-w);
				if(j<l-1) graph.addEdge(src, 0, src+w);
				if(k>0) graph.addEdge(src, 0, src-w*l);
				if(k<h-1) graph.addEdge(src, 0, src+w*l);
			}
		}
	}
	
	printf("Nodes: %d\n", w*h*l);

	printf("Creating host link...\n");
	HostLink hostLink;

	printf("Mapping...\n");
	graph.mapVerticesToDRAM = true;
	graph.mapEdgesToDRAM = true;
	MAP(graph);

	// read edge weights and create edges
	int32_t rootNode = 2*w+w/2; // TODO: source id from arg
	for(int32_t sk=0; sk<h; sk++)
		for(int32_t sj=0; sj<l; sj++)
			for(int32_t si=0; si<w; si++) {
				PDeviceId s = (sk*l+sj)*w+si;
				DenQSeismic3DState* dev = &graph.devices[s]->state;
				dev->x = si;
				dev->y = sj;
				dev->z = sk;
				dev->isSource = (s==rootNode);
				
				for(int32_t rk=-R; rk<=R; rk++)
					for(int32_t rj=-R; rj<=R; rj++)
						for(int32_t ri=-R; ri<=R; ri++) {
							int32_t i = ri+R;
							int32_t j = rj+R;
							int32_t k = rk+R;
							int32_t index = makeIndex(i, j, k);

							int32_t di = (si+ri+sw)%sw;
							int32_t dj = (sj+rj+sl)%sl;
							int32_t dk = (sk+rk+sh)%sh;
							
							dev->changed[index] = d[(dk*sl+dj)*sw+di];
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
		PMessage<None, DenQSeismic3DMessage> msg;
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
