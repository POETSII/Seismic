// SPDX-License-Identifier: BSD-2-Clause

#ifndef _ASP_H_
#define _ASP_H_

#include <POLite.h>

//#define EDGE_TYPE float
#define EDGE_TYPE uint32_t
#define USE_WEIGHTS 1
#define FIXED_WEIGHTS 0

#define INF 0xffffff

struct ASPMessage {
	uint32_t nodeId;
	EDGE_TYPE dist;
};

struct ASPState {
	uint32_t nodeId;
	int32_t root;

	EDGE_TYPE dist;
	uint8_t updated;
};

struct ASPDevice : PDevice<ASPState, EDGE_TYPE, ASPMessage> {
	// Called once by POLite at start of execution
	inline void init() {
		s->dist = INF;
		s->updated = 0;
		if(s->root)
			*readyToSend = Pin(0);
	}

	// Send handler
	inline void send(volatile ASPMessage* msg) {
		if(s->root) {
			s->dist = 0;
			msg->nodeId = 0;
			msg->dist = 0;
			s->root = 0;
		}
		else {
			if(s->updated) {
				msg->nodeId = 0;
				msg->dist = s->dist;
				s->updated = 0;
			}
		}
		if(s->updated)
			*readyToSend = Pin(0);
		else
			*readyToSend = No;
	}

	// Receive handler
	inline void recv(ASPMessage* msg, EDGE_TYPE* edge) {
#if USE_WEIGHTS
		EDGE_TYPE newDist = msg->dist+*edge;
#else
		EDGE_TYPE newDist = msg->dist+1;
#endif
		if(newDist < s->dist) {
			s->dist = newDist;
			s->updated = 1;
		}
		if(s->updated)
			*readyToSend = Pin(0);
	}

	// Called by POLite on idle event
	inline bool step() {
		*readyToSend = No;
		return false;
	}

	// Optionally send message to host on termination
	inline bool finish(volatile ASPMessage* msg) {
		msg->dist = s->dist;
		msg->nodeId = s->nodeId;
		return true;
	}

};

#endif
