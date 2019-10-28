#ifndef _NET_H_
#define _NET_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

float lerp(float x0, float x1, float s) {
	return x0 * (1.0-s) + x1 * s;
}

struct SourceData {
	int32_t w, h;
	float* d;
	
	inline void setd(int x, int y, float v) {
		d[y*w+x] = v;
	}

	float getd(int x, int y) {
		return d[y*w+x];
	}
	
	float get(float x, float y) {
		int32_t x0 = (int32_t)floor(x);
		float sx = x-x0;
		int32_t y0 = (int32_t)floor(y);
		float sy = y-y0;
		float d00 = getd(x0, y0);
		float d01 = getd(x0, y0+1);
		float d10 = getd(x0+1, y0);
		float d11 = getd(x0+1, y0+1);
		return lerp(lerp(d00, d01, sy), lerp(d10, d11, sy), sx);
	}
	
	float line(float x0, float y0, float x1, float y1) {
		float dx = x1-x0;
		float dy = y1-y0;
		float dist = sqrt(dx*dx+dy*dy);
		int32_t steps = (int32_t)(dist*2.0);
		float sum = 0;
		for(int32_t i=0; i<=steps; i++) {
			float s = i / (float)steps;
			float x = lerp(x0, x1, s);
			float y = lerp(y0, y1, s);
			sum += get(x, y);
		}
		return (sum / (float)steps) * dist;
	}
};

int32_t readIntBE(FILE* in) {
	unsigned char b;
	int32_t x = 0;
	for(int32_t d=0; d<4; d++) {
		fread(&b, 1, 1, in);
		x = (x<<8) | b;
	}
	return x;
}

void readSourceData(FILE* in, SourceData* src, float bias, float scale) {
	src->w = readIntBE(in);
	src->h = readIntBE(in);
	src->d = (float*) calloc(src->w*src->h, sizeof(float));
	unsigned char rgb;
	for(int32_t y=0; y<src->h; y++) {
		for(int32_t x=0; x<src->w; x++) {
			fread(&rgb, 1, 1, in);
			src->setd(x, y, (1.0-rgb/255.0)*scale+bias);
		}
	}
}

struct Fanout {
	int32_t r;
	int32_t count;
	bool* e;
	inline void sete(int32_t ri, int32_t rj, bool v) {
		e[rj*(r+1)+ri] = v;
	}
	inline bool gete(int32_t ri, int32_t rj) {
		return e[rj*(r+1)+ri];
	}
};

const int32_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113};

bool acceptFSEdge(int32_t x, int32_t n) {
	if(n==0)
		return x<2;
	for(int32_t pi=0; primes[pi]<x; pi++) {
		int32_t p = primes[pi];
		if(x%p==0 && n%p==0)
			return false;
	}
	return true;
}
	
int32_t forwardStar(Fanout* f, int32_t r) {
	f->r = r;
	f->e = (bool*) calloc((r+1)*(r+1), sizeof(bool));
	f->sete(0, 0, false);
	int32_t count = 0;
	for(int32_t x=1; x<=r; x++) {
		for(int32_t i=0; i<x; i++) {
			bool a = acceptFSEdge(x, i);
			if(a) count++;
			f->sete(x, i, a);
			f->sete(i, x, a);
			if(i==0)
				f->sete(x, x, a);
		}
	}
	f->count = count*8;
	return f->count;
}

void printFanout(FILE* out, Fanout* f) {
	fprintf(out, "%d %d\n", f->r, f->count);
	for(int32_t rj=0; rj<=f->r; rj++) {
		for(int32_t ri=0; ri<=f->r; ri++) {
			fprintf(out, f->gete(ri, rj) ? "1 " : "0 ");
		}
		fprintf(out, "\n");
	}
}

struct Net {
	int32_t w, h, step;
	inline int32_t nodeIndex(int32_t i, int32_t j) {
		return j*w + i;
	}
	inline float itox(int32_t i) {
		return i*step + step/2.0;
	}
	inline float jtoy(int32_t j) {
		return j*step + step/2.0;
	}
};

int32_t countEdges(Net* net, Fanout* fanout) {
	int32_t count = 0;
	int32_t r = fanout->r;
	for(int32_t sj=0; sj<net->h; sj++) {
		for(int32_t si=0; si<net->w; si++) {
			for(int32_t rj=-r; rj<=r; rj++)
				for(int32_t ri=-r; ri<=r; ri++) {
					if(fanout->gete(abs(ri), abs(rj))) {
						int32_t di = si+ri;
						int32_t dj = sj+rj;
						if(di<0 || dj<0 || di>=net->w || dj>=net->h)
							continue;
						count++;
					}
				}
		}
	}
	return count;
}

void writeNet(FILE* out, SourceData* src, int step, Fanout* fanout) {
	Net net;
	net.w = src->w / step;
	net.h = src->h / step;
	net.step = step;

	fprintf(out, "%d %d %d ", net.w, net.h, step);
	printFanout(out, fanout);
	fprintf(out, "%d\n", countEdges(&net, fanout));
	
	int32_t r = fanout->r;
	for(int32_t sj=0; sj<net.h; sj++) {
		for(int32_t si=0; si<net.w; si++) {
			int32_t s = net.nodeIndex(si, sj);
			float sx = net.itox(si);
			float sy = net.jtoy(sj);
			for(int32_t rj=-r; rj<=r; rj++)
				for(int32_t ri=-r; ri<=r; ri++) {
					if(fanout->gete(abs(ri), abs(rj))) {
						int32_t di = si+ri;
						int32_t dj = sj+rj;
						if(di<0 || dj<0 || di>=net.w || dj>=net.h)
							continue;
						int32_t d = net.nodeIndex(di, dj);
						float dx = net.itox(di);
						float dy = net.jtoy(dj);

						float x = src->line(sx, sy, dx, dy);
						fprintf(out, "%.3f ", x);
					}
				}
			fprintf(out, "\n");
		}
		printf("\r%d of %d", (sj+1)*net.w, net.h*net.w);
		fflush(stdout);
	}
	printf("\nDone\n");
}

#endif
