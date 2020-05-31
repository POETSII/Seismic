// SPDX-License-Identifier: BSD-2-Clause
#ifndef _QSEISMIC_H_
#define _QSEISMIC_H_

//#define POLITE_DUMP_STATS
//#define POLITE_COUNT_MSGS

#include <POLite.h>

#define R 5
#define D (R*2+1)
#define MID (R*D+R)

int32_t makeIndex(int32_t i, int32_t j) {
	return i*D+j;
}

// Vertex state
struct QSeismicState {
	int32_t x, y;
	bool isSource;
	int32_t w[D*D];
	int32_t pending;
	bool changed[D*D];
	int32_t dist[D*D];
	int32_t parentx, parenty;
};

struct QSeismicMessage {
	int32_t x, y;
	int32_t dist;
};

// Vertex behaviour
struct QSeismicDevice : PDevice<QSeismicState, None, QSeismicMessage> {

	inline void init() {
		for(int32_t i=0; i<D; i++)
			for(int32_t j=0; j<D; j++) {
				int32_t index = makeIndex(i, j);
				s->changed[index] = false;
				s->dist[index] = -1;
			}
		s->pending = 0;
		s->parentx = -1;
		s->parenty = -1;
		if(s->isSource) {
			s->dist[MID] = 0;
			s->changed[MID] = true;
			s->pending = 1;
			*readyToSend = Pin(0);
		}
		else {
			*readyToSend = No;
		}
	}

	inline void send(volatile QSeismicMessage* msg) {
		for(int32_t i=0; i<D; i++)
			for(int32_t j=0; j<D; j++) {
				int32_t index = makeIndex(i, j);
				if(s->changed[index]) {
					msg->x = s->x + (i-R);
					msg->y = s->y + (j-R);
					msg->dist = s->dist[index];
					s->changed[index] = false;
					s->pending--;
					*readyToSend = s->pending>0 ? Pin(0) : No;
					return;
				}
			}
		*readyToSend = No;
	}

	inline bool updateDist(int32_t index, int32_t newDist) {
		if(s->dist[index]<0 || s->dist[index]>newDist) {
			s->dist[index] = newDist;
			if(!s->changed[index]) {
				s->changed[index] = true;
				s->pending++;
			}
			return true;
		}
		else
			return false;
	}
	
	inline void recv(QSeismicMessage* msg, None* weight) {
		int32_t i = (msg->x - s->x) + R;
		int32_t j = (msg->y - s->y) + R;
		if(i>=0 && i<D && j>=0 && j<D) {
			int32_t index = makeIndex(i, j);
			if(updateDist(index, msg->dist)) {
				if(s->w[index]>0) {
					if(updateDist(MID, msg->dist + s->w[index])) {
						s->parentx = msg->x;
						s->parenty = msg->y;
					}
				}
			}
		}
		*readyToSend = No;
	}

	inline bool step() {
		if(s->pending>0) {
			*readyToSend = Pin(0);
			return true;
		}
		else
			return false;
	}
	
	inline bool finish(volatile QSeismicMessage* msg) {
		msg->x = s->parentx;
		msg->y = s->parenty;
		msg->dist = s->dist[MID];
		return true;
	}
};

#endif
