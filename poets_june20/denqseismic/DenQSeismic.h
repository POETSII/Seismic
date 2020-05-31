// SPDX-License-Identifier: BSD-2-Clause
#ifndef _DENQSEISMIC_H_
#define _DENQSEISMIC_H_

//#define POLITE_DUMP_STATS
//#define POLITE_COUNT_MSGS

#include <POLite.h>
#include <math.h>

#define R 5
#define D (R*2+1)
#define MID (R*D+R)

#define LINE_STEP 1.0
#define BIAS 0.25
#define SCALE 1.0
#define PREC 1000.0

inline int32_t makeIndex(int32_t i, int32_t j) {
	return i*D+j;
}

inline float lerp(float x0, float x1, float s) {
	return x0 * (1-s) + x1 * s;
}

// Vertex state
struct DenQSeismicState {
	int32_t x, y;
	bool isSource;
	int32_t w[D*D];
	int32_t pending;
	uint8_t changed[D*D]; // also used as density before init()
	int32_t dist[D*D];
	int32_t parentx, parenty;
};

struct DenQSeismicMessage {
	int32_t x, y;
	int32_t dist;
};

// Vertex behaviour
struct DenQSeismicDevice : PDevice<DenQSeismicState, None, DenQSeismicMessage> {

	inline float d(int32_t i, int32_t j) {
		return (1.0-s->changed[makeIndex(i, j)]/255.0)*SCALE+BIAS;
	}
	
	inline float get(float x, float y) {
		int32_t x0 = (int32_t) floor(x);
		int32_t x1 = x0+1;
		if(x1>=D) x1 = x0;
		float sx = x-x0;
		int32_t y0 = (int32_t) floor(y);
		int32_t y1 = y0+1;
		if(y1>=D) y1 = y0;
		float sy = y-y0;
		float d00 = d(x0, y0);
		float d01 = d(x0, y1);
		float d10 = d(x1, y0);
		float d11 = d(x1, y1);
		return lerp(lerp(d00, d01, sy), lerp(d10, d11, sy), sx);
	}
	
	inline float line(float x0, float y0, float x1, float y1) {
		float dx = x1-x0;
		float dy = y1-y0;
		float dist = sqrt(dx*dx+dy*dy);
		int32_t steps = (int32_t)(dist*LINE_STEP);
		float sum = 0;
		float prev = get(x0, y0);
		for(int32_t i=1; i<=steps; i++) {
			float s = i / (float)steps;
			float x = lerp(x0, x1, s);
			float y = lerp(y0, y1, s);
			float v = get(x, y);
			sum += (prev+v)/2;
			prev = v;
		}
		return sum * dist / (float)steps;
	}
	
	inline void init() {
		for(int32_t i=0; i<D; i++)
			for(int32_t j=0; j<D; j++) {
				int32_t index = makeIndex(i, j);
				if(i!=R || j!=R) {
					float x = line(R, R, i, j);
					s->w[index] = round(x*PREC);
				}
				else {
					s->w[index] = 0;
				}
			}
		
		for(int32_t i=0; i<D; i++)
			for(int32_t j=0; j<D; j++) {
				int32_t index = makeIndex(i, j);
				s->changed[index] = 0;
				s->dist[index] = -1;
			}
		s->pending = 0;
		s->parentx = -1;
		s->parenty = -1;
		if(s->isSource) {
			s->dist[MID] = 0;
			s->changed[MID] = 1;
			s->pending = 1;
			*readyToSend = Pin(0);
		}
		else {
			*readyToSend = No;
		}
	}

	inline void send(volatile DenQSeismicMessage* msg) {
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
				s->changed[index] = 1;
				s->pending++;
			}
			return true;
		}
		else
			return false;
	}
	
	inline void recv(DenQSeismicMessage* msg, None* weight) {
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
	
	inline bool finish(volatile DenQSeismicMessage* msg) {
		msg->x = s->parentx;
		msg->y = s->parenty;
		msg->dist = s->dist[MID];
		return true;
	}
};

#endif
