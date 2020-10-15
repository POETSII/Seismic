// SPDX-License-Identifier: BSD-2-Clause
#ifndef _TRAVERSE_H_
#define _TRAVERSE_H_

#define POLITE_MAX_FANOUT 160
#define ASYNC 1
#define IGNORE_WEIGHTS 0
#define COUNT_VISITS 0

//#define POLITE_DUMP_STATS
//#define POLITE_COUNT_MSGS

#include <POLite.h>

// Vertex state
struct SSSPState {
	int32_t node;
	bool isSource;
	bool changed;
	int32_t hops;
	int32_t dist;
	#if COUNT_VISITS
	int32_t visits;
	#endif
};

struct SSSPMessage {
	int32_t node;
	int32_t hops;
	int32_t dist;
	#if COUNT_VISITS
	int32_t visits;
	#endif
};

// Vertex behaviour
struct SSSPDevice : PDevice<SSSPState, int32_t, SSSPMessage> {

	inline void init() {
		/*
			//From the host:
			s->node = nodeId;
			s->isSource = (nodeId==rootNode);
			s->hops = 0;
			s->dist = s->isSource ? 0 : -1;
		*/
		#if COUNT_VISITS
		s->visits = 0;
		#endif
		s->changed = false;
		*readyToSend = s->isSource ? Pin(0) : No;
	}

	inline void send(volatile SSSPMessage* msg) {
		msg->node = s->node;
		msg->hops = s->hops;
		msg->dist = s->dist;
		s->changed = false;
		*readyToSend = No;
	}

	inline void recv(SSSPMessage* msg, int32_t* weight) {
		#if IGNORE_WEIGHTS
		int32_t newDist = msg->dist + 1;
		#else
		int32_t newDist = msg->dist + *weight;
		#endif
		if(s->dist<0 || newDist<s->dist) {
			s->hops = msg->hops + 1;
			s->dist = newDist;
			#if COUNT_VISITS
			s->visits++;
			#endif
			#if ASYNC
			*readyToSend = Pin(0);
			#else
			s->changed = true;
			#endif
		}
	}

	inline bool step() {
		#if ASYNC
		return false;
		#else
		if (s->changed) {
			*readyToSend = Pin(0);
			return true;
		}
		else {
			return false;
		}
		#endif
	}
	
	inline bool finish(volatile SSSPMessage* msg) {
		msg->node = s->node;
		msg->hops = s->hops;
		msg->dist = s->dist;
		#if COUNT_VISITS
		msg->visits = s->visits;
		#endif
		return true;
	}
};

#endif
