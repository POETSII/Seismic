#ifndef _NET_H_
#define _NET_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define PREC 1000.0
#define LINE_STEP 1.0

double lerp(double x0, double x1, double s) {
	return x0 * (1.0-s) + x1 * s;
}

struct SourceData {
	int32_t w, h;
	double* d;
	
	inline void setd(int32_t x, int32_t y, double v) {
		d[y*w+x] = v;
	}

	double getd(int32_t x, int32_t y) {
		return d[y*w+x];
	}
	
	double get(double x, double y) {
		int32_t x0 = (int32_t)floor(x);
		int32_t x1 = x0+1;
		if(x1>=w) x1 = x0;
		double sx = x-x0;
		int32_t y0 = (int32_t)floor(y);
		int32_t y1 = y0+1;
		if(y1>=h) y1 = y0;
		double sy = y-y0;
		double d00 = getd(x0, y0);
		double d01 = getd(x0, y1);
		double d10 = getd(x1, y0);
		double d11 = getd(x1, y1);
		return lerp(lerp(d00, d01, sy), lerp(d10, d11, sy), sx);
	}
	
	double line(double x0, double y0, double x1, double y1) {
		double dx = x1-x0;
		double dy = y1-y0;
		double dist = sqrt(dx*dx+dy*dy);
		int32_t steps = (int32_t)(dist*LINE_STEP);
		double sum = 0;
		double prev = get(x0, y0);
		for(int32_t i=1; i<=steps; i++) {
			double s = (double)i / (double)steps;
			double x = lerp(x0, x1, s);
			double y = lerp(y0, y1, s);
			double v = get(x, y);
			sum += (prev+v)/2.0;
			prev = v;
		}
		return sum * dist / (double)steps;
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

void readSourceData(FILE* in, SourceData* src, double bias, double scale) {
	src->w = readIntBE(in);
	src->h = readIntBE(in);
	src->d = (double*) calloc(src->w*src->h, sizeof(double));
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

int32_t fullFanout(Fanout* f, int32_t r) {
	f->r = r;
	f->e = (bool*) calloc((r+1)*(r+1), sizeof(bool));
	for(int32_t j=0; j<=r; j++) {
		for(int32_t i=0; i<=r; i++) {
			f->sete(i, j, true);
		}
	}
	f->sete(0, 0, false);
	f->count = (2*r+1)*(2*r+1)-1;
	return f->count;
}

int32_t bresenhamFanout(Fanout* f, int32_t r) {
	if(r<2)
		return fullFanout(f, r);
	
	f->r = r;
	f->e = (bool*) calloc((r+1)*(r+1), sizeof(bool));

	for(int32_t x=0; x<=r; x++)
		for(int32_t y=0; y<=r; y++) {
				f->sete(x, y, x==1 || y==1);
			}
	f->sete(0, 0, false);
	
	f->count = (2*r+1)*6-10;
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
	inline double itox(int32_t i) {
		return i*step + step/2.0;
	}
	inline double jtoy(int32_t j) {
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
			double sx = net.itox(si);
			double sy = net.jtoy(sj);
			for(int32_t rj=-r; rj<=r; rj++)
				for(int32_t ri=-r; ri<=r; ri++) {
					if(fanout->gete(abs(ri), abs(rj))) {
						int32_t di = si+ri;
						int32_t dj = sj+rj;
						if(di<0 || dj<0 || di>=net.w || dj>=net.h)
							continue;
						int32_t d = net.nodeIndex(di, dj);
						double dx = net.itox(di);
						double dy = net.jtoy(dj);

						int32_t x = round(src->line(sx, sy, dx, dy)*PREC);
						fprintf(out, "%d ", x);
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
