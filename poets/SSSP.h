// SPDX-License-Identifier: BSD-2-Clause
#ifndef _SSSP_H_
#define _SSSP_H_

//#define POLITE_DUMP_STATS
//#define POLITE_COUNT_MSGS

#include <POLite.h>

// Vertex state
struct SSSPState {
	int32_t node;
	bool isSource;
	bool changed;
	int32_t parent;
	int32_t dist;
};

struct SSSPMessage {
	int32_t node;
	int32_t from;
	int32_t dist;
};

// Vertex behaviour
struct SSSPDevice : PDevice<SSSPState, int32_t, SSSPMessage> {

	inline void init() {
		*readyToSend = s->isSource ? Pin(0) : No;
	}

	inline void send(volatile SSSPMessage* msg) {
		msg->node = s->node;
		msg->from = s->node;
		msg->dist = s->dist;
		*readyToSend = No;
	}

	inline void recv(SSSPMessage* msg, int32_t* weight) {
		int32_t newDist = msg->dist + *weight;
		if(s->dist<0 || newDist<s->dist) {
			s->parent = msg->from;
			s->dist = newDist;
			s->changed = true;
		}
	}

	inline bool step() {
		if (s->changed) {
			s->changed = false;
			*readyToSend = Pin(0);
		return true;
		}
		else
			return false;
	}
	
	inline bool finish(volatile SSSPMessage* msg) {
		msg->node = s->node;
		msg->from = s->parent;
		msg->dist = s->dist;
		return true;
	}
};

#endif
