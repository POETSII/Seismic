// SPDX-License-Identifier: BSD-2-Clause
#ifndef _DENQSEISMIC3D_H_
#define _DENQSEISMIC3D_H_

#define POLITE_MAX_FANOUT 32

//#define POLITE_DUMP_STATS
//#define POLITE_COUNT_MSGS

#include <POLite.h>

#define R 1
#define D (R*2+1)
#define MID ((R*D+R)*D+R)

#define LINE_STEP 1.0
#define BIAS 0.25
#define SCALE 1.0
#define PREC 1000.0

const float precalcDist[8] = {
	0.0, 1.0,
	1.0, 1.41421356237,
	0.0, 1.0,
	1.0, 1.41421356237
};
static_assert(sizeof(precalcDist)/sizeof(float)==(R+1)*(R+1)*(R+1), "precalcDist size is not compatible with R");

inline int32_t makeIndex(int32_t i, int32_t j, int32_t k) {
	return (k*D+i)*D+j;
}

inline float lerp(float x0, float x1, float s) {
	return x0 * (1-s) + x1 * s;
}

inline float getDist(int32_t dx, int32_t dy, int32_t dz) {
	return precalcDist[(abs(dz)*(R+1)+abs(dy))*(R+1)+abs(dx)];
}

// Vertex state
struct DenQSeismic3DState {
	int32_t x, y, z;
	bool isSource;
	int32_t w[D*D*D];
	int32_t pending;
	uint8_t changed[D*D*D]; // also used as density before init()
	int32_t dist[D*D*D];
	int32_t parentx, parenty, parentz;
};

struct DenQSeismic3DMessage {
	int32_t x, y, z;
	int32_t dist;
};

// Vertex behaviour
struct DenQSeismic3DDevice : PDevice<DenQSeismic3DState, None, DenQSeismic3DMessage> {

	inline float d(int32_t i, int32_t j, int32_t k) {
		return (1.0-s->changed[makeIndex(i, j, k)]/255.0)*SCALE+BIAS;
	}
	
	inline float get(float x, float y, float z) {
		int32_t x0 = (int32_t) x; // floor
		int32_t x1 = x0+1;
		if(x1>=D) x1 = x0;
		float sx = x-x0;
		int32_t y0 = (int32_t) y; // floor
		int32_t y1 = y0+1;
		if(y1>=D) y1 = y0;
		float sy = y-y0;
		int32_t z0 = (int32_t) z; // floor
		int32_t z1 = z0+1;
		if(z1>=D) z1 = z0;
		float sz = z-z0;
		float d000 = d(x0, y0, z0);
		float d010 = d(x0, y1, z0);
		float d100 = d(x1, y0, z0);
		float d110 = d(x1, y1, z0);
		float d001 = d(x0, y0, z1);
		float d011 = d(x0, y1, z1);
		float d101 = d(x1, y0, z1);
		float d111 = d(x1, y1, z1);
		return lerp(
				lerp(lerp(d000, d010, sy), lerp(d100, d110, sy), sx),
				lerp(lerp(d001, d011, sy), lerp(d101, d111, sy), sx),
				sz
			);
	}
	
	inline float line(float x0, float y0, float z0, float x1, float y1, float z1) {
		float dx = x1-x0;
		float dy = y1-y0;
		float dz = z1-z0;
		float dist = getDist(dx, dy, dz); //sqrt(dx*dx+dy*dy+dz*dz);
		int32_t steps = (int32_t)(dist*LINE_STEP);
		float sum = 0;
		float prev = get(x0, y0, z0);
		for(int32_t i=1; i<=steps; i++) {
			float s = i / (float)steps;
			float x = lerp(x0, x1, s);
			float y = lerp(y0, y1, s);
			float z = lerp(z0, z1, s);
			float v = get(x, y, z);
			sum += (prev+v)/2;
			prev = v;
		}
		return sum * dist / (float)steps;
	}
	
	inline void init() {
		for(int32_t k=0; k<D; k++)
			for(int32_t j=0; j<D; j++)
				for(int32_t i=0; i<D; i++) {
					int32_t index = makeIndex(i, j, k);
					if(i!=R || j!=R) {
						float x = line(R, R, R, i, j, k);
						s->w[index] = (int32_t)(x*PREC+0.5); // round
					}
					else {
						s->w[index] = 0;
					}
			}
		
		for(int32_t k=0; k<D; k++)
			for(int32_t j=0; j<D; j++)
				for(int32_t i=0; i<D; i++) {
					int32_t index = makeIndex(i, j, k);
					s->changed[index] = 0;
					s->dist[index] = -1;
				}
		s->pending = 0;
		s->parentx = -1;
		s->parenty = -1;
		s->parentz = -1;
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

	inline void send(volatile DenQSeismic3DMessage* msg) {
		for(int32_t k=0; k<D; k++)
			for(int32_t j=0; j<D; j++)
				for(int32_t i=0; i<D; i++) {
					int32_t index = makeIndex(i, j, k);
					if(s->changed[index]) {
						msg->x = s->x + (i-R);
						msg->y = s->y + (j-R);
						msg->z = s->z + (k-R);
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
	
	inline void recv(DenQSeismic3DMessage* msg, None* weight) {
		int32_t i = (msg->x - s->x) + R;
		int32_t j = (msg->y - s->y) + R;
		int32_t k = (msg->z - s->z) + R;
		if(i>=0 && i<D && j>=0 && j<D && k>=0 && k<D) {
			int32_t index = makeIndex(i, j, k);
			if(updateDist(index, msg->dist)) {
				if(s->w[index]>0) {
					if(updateDist(MID, msg->dist + s->w[index])) {
						s->parentx = msg->x;
						s->parenty = msg->y;
						s->parentz = msg->z;
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
	
	inline bool finish(volatile DenQSeismic3DMessage* msg) {
		msg->x = s->x;
		msg->y = s->y;
		msg->z = s->z;
		msg->dist = s->dist[MID];
		return true;
	}
};

#endif
