// SPDX-License-Identifier: BSD-2-Clause
#ifndef _LOADGRAPH_H_
#define _LOADGRAPH_H_

#include <assert.h>
#include <vector>

#define IGNORE_WEIGHTED_NODES 1
#define FLAT_WEIGHTS 0

class LoadEdge {
public:
	size_t dst;
	int32_t cost;
	
	LoadEdge(uint32_t d, int32_t c) : dst(d), cost(c) {}
};

class LoadNode {
public:
	size_t index;
	std::vector<LoadEdge> edges;
	
	LoadNode(size_t i) : index(i) {}
	
	size_t addEdge(size_t dst, int32_t c) {
		LoadEdge e(dst, c);
		size_t edge = edges.size();
		edges.push_back(e);
		return edge;
	}
};

class LoadGraph {
public:
	size_t numNodes;
	std::vector<LoadNode> nodes;
	uint8_t *ignoreNodes;
	int32_t rootNode; // SSSP source id
	
	size_t addNode() {
		LoadNode node(nodes.size());
		nodes.push_back(node);
		return node.index;
	}
	
	size_t addEdge(size_t src, size_t dst, int32_t cost) {
		return nodes[src].addEdge(dst, cost);
	}
	
	void load(const char* filename) {
		// Input format: GitHub/ashurrafiev/ggraph
		FILE* in = fopen(filename, "rt");
		if(in==NULL) {
			fprintf(stderr, "Can't read file: %s\n", filename);
			exit(1);
		}

		size_t totalNodes, totalEdges, maxFanout;
		assert(fscanf(in, "%zu %zu %zu", &totalNodes, &totalEdges, &maxFanout)==3);
		
		// create graph
		printf("Loading...\n");
		numNodes = totalNodes;
		for(size_t i = 0; i < totalNodes; i++) {
			addNode();
		}
		
		printf("Nodes: %zu\nEdges: %zu\n", totalNodes, totalEdges);
		printf("Fanout: %zu\n", maxFanout);
		
		ignoreNodes = (uint8_t*) calloc(totalNodes, sizeof(uint8_t));
		
		// read edge weights and create edges
		for(size_t i = 0; i < totalNodes; i++) {
			int32_t ncost; // ignored
			size_t edges;
			assert(fscanf(in, "%d %zu", &ncost, &edges)==2);
			#if IGNORE_WEIGHTED_NODES
			if(ncost) ignoreNodes[i] = 1;
			#endif
			for(size_t j=0; j<edges; j++) {
				size_t dst;
				int32_t w;
				assert(fscanf(in, "%zu %d", &dst, &w)==2);
				#if FLAT_WEIGHTS
				if(w) w = 1;
				#endif
				addEdge(i, dst, w);
				totalEdges--;
			}
		}
		
		rootNode = 0;
		{
			char key[1024];
			while(fscanf(in, "%s", key)>0) {
				if(key[0]=='#')
					fgets(key, 1024, in)==key; // -Wunused-result
				else if(key[0]!='$')
					break;
				else {
					if(!strcmp(key, "$rootNode"))
						assert(fscanf(in, "%d", &rootNode)==1);
					else
						fgets(key, 1024, in)==key; // -Wunused-result
				}
			}
		}
		
		fclose(in);
		assert(totalEdges==0);
	}
};

#endif
