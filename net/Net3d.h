#ifndef _NET3D_H_
#define _NET3D_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>

float lerp(float x0, float x1, float s) {
	return x0 * (1.0-s) + x1 * s;
}

struct SourceData3D {
	int32_t w, l, h;
	float* d;
	
	inline void setd(int x, int y, int z, float v) {
		d[(z*l+y)*w+x] = v;
	}

	float getd(int x, int y, int z) {
		return d[(z*l+y)*w+x];
	}
	
	float get(float x, float y, float z) {
		int32_t x0 = (int32_t)floor(x) & (w-1);
		int32_t x1 = (x0+1) & (w-1);
		float sx = x-x0;
		int32_t y0 = (int32_t)floor(y) & (l-1);
		int32_t y1 = (y0+1) & (l-1);
		float sy = y-y0;
		int32_t z0 = (int32_t)floor(z) & (h-1);
		int32_t z1 = (z0+1) & (h-1);
		float sz = z-z0;
		float d000 = getd(x0, y0, z0);
		float d010 = getd(x0, y1, z0);
		float d100 = getd(x1, y0, z0);
		float d110 = getd(x1, y1, z0);
		float d001 = getd(x0, y0, z1);
		float d011 = getd(x0, y1, z1);
		float d101 = getd(x1, y0, z1);
		float d111 = getd(x1, y1, z1);
		return lerp(
			lerp(lerp(d000, d010, sy), lerp(d100, d110, sy), sx),
			lerp(lerp(d001, d011, sy), lerp(d101, d111, sy), sx),
			sz);
	}
	
	float line(float x0, float y0, float z0, float x1, float y1, float z1) {
		float dx = x1-x0;
		float dy = y1-y0;
		float dz = z1-z0;
		float dist = sqrt(dx*dx+dy*dy+dz*dz);
		int32_t steps = (int32_t)(dist*2.0);
		float sum = 0;
		for(int32_t i=0; i<=steps; i++) {
			float s = i / (float)steps;
			float x = lerp(x0, x1, s);
			float y = lerp(y0, y1, s);
			float z = lerp(z0, z1, s);
			sum += get(x, y, z);
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

void readSourceData(FILE* in, SourceData3D* src, float bias, float scale) {
	src->w = readIntBE(in);
	src->l = readIntBE(in);
	src->h = readIntBE(in);
	src->d = (float*) calloc(src->w*src->l*src->h, sizeof(float));
	unsigned char rgb;
	for(int32_t x=0; x<src->w; x++) {
		for(int32_t y=0; y<src->l; y++) {
			for(int32_t z=0; z<src->h; z++) {
				fread(&rgb, 1, 1, in);
				src->setd(x, y, z, (1.0-rgb/255.0)*scale+bias);
			}
		}
	}
}

struct Fanout3D {
	int32_t r;
	int32_t count;
	bool* e;
	inline void sete(int32_t ri, int32_t rj, int32_t rk, bool v) {
		e[(rk*(r+1)+rj)*(r+1)+ri] = v;
	}
	inline bool gete(int32_t ri, int32_t rj, int32_t rk) {
		return e[(rk*(r+1)+rj)*(r+1)+ri];
	}
};

int32_t countFanout(Fanout3D* f) {
	int32_t count = 0;
	for(int32_t x=0; x<=f->r; x++)
		for(int32_t y=0; y<=f->r; y++)
			for(int32_t z=0; z<=f->r; z++) {
				if(f->gete(x, y, z)) {
					int32_t n = 8;
					if(x==0) n >>= 1;
					if(y==0) n >>= 1;
					if(z==0) n >>= 1;
					count += n;
				}
			}
	f->count = count;
	return count;
}

int32_t forwardStar3D(Fanout3D* f, int32_t r, bool spherical) {
	f->r = r;
	f->e = (bool*) calloc((r+1)*(r+1)*(r+1), sizeof(bool));

	for(int32_t x=0; x<=r; x++)
		for(int32_t y=0; y<=r; y++)
			for(int32_t z=0; z<=r; z++) {
				f->sete(x, y, z, spherical ? ceil(sqrt(x*x+y*y+z*z))<=r : true);
			}
	for(int32_t x=0; x<=r; x++)
		for(int32_t y=0; y<=r; y++)
			for(int32_t z=0; z<=r; z++) {
				for(int32_t m=2; m<=r; m++)
					if(x*m<=r && y*m<=r && z*m<=r)
							f->sete(x*m, y*m, z*m, false);
			}
	f->sete(0, 0, 0, false);
	
	return countFanout(f);
}

int32_t bresenhamFanout3D(Fanout3D* f, int32_t r) {
	f->r = r;
	f->e = (bool*) calloc((r+1)*(r+1)*(r+1), sizeof(bool));

	for(int32_t x=0; x<=r; x++)
		for(int32_t y=0; y<=r; y++)
			for(int32_t z=0; z<=r; z++) {
				f->sete(x, y, z, (x<2 && y<2) || (x<2 && z<2) || (y<2 && z<2));
			}
	f->sete(0, 0, 0, false);
	
	return countFanout(f);
}

void printFanout(FILE* out, Fanout3D* f) {
	fprintf(out, "%d %d\n", f->r, f->count);
	for(int32_t rk=0; rk<=f->r; rk++) {
		fprintf(out, "\n");
		for(int32_t rj=0; rj<=f->r; rj++) {
			for(int32_t ri=0; ri<=f->r; ri++) {
				fprintf(out, f->gete(ri, rj, rk) ? "1 " : "0 ");
			}
			fprintf(out, "\n");
		}
	}
	fprintf(out, "\n");
}

struct Net3D {
	int32_t w, l, h, stepx, stepy, stepz;
	inline int32_t nodeIndex(int32_t i, int32_t j, int32_t k) {
		return (k*l+j)*w + i;
	}
	inline float itox(int32_t i) {
		return i*stepx;
	}
	inline float jtoy(int32_t j) {
		return j*stepy;
	}
	inline float ktoz(int32_t k) {
		return k*stepz;
	}
};

struct Progress {
	int32_t prog, msgCount, max, step;
	struct timeval start;
};

void start(Progress* p, int32_t max, int32_t step) {
	p->prog = 0;
	p->msgCount = 0;
	p->max = max;
	p->step = step;
	gettimeofday(&p->start, NULL);
}

void next(Progress* p) {
	p->prog++;
	if(p->prog==p->step) {
		p->msgCount++;
		int32_t count = p->msgCount*p->step;
		struct timeval t, diff;
		gettimeofday(&t, NULL);
		timersub(&t, &p->start, &diff);
		double duration = (double) diff.tv_sec + (double) diff.tv_usec / 1000000.0;
		double eta = duration * p->max / (double)count - duration;
		printf("\r%d of %d, ETA: %.0lf sec", count, p->max, eta);
		fflush(stdout);
		p->prog = 0;
	}
}

void writeNet(FILE* out, SourceData3D* src, int32_t stepx, int32_t stepy, int32_t stepz, Fanout3D* fanout) {
	Net3D net;
	net.w = src->w / stepx;
	net.l = src->l / stepy;
	net.h = src->h / stepz;
	net.stepx = stepx;
	net.stepy = stepy;
	net.stepz = stepz;

	fprintf(out, "T3.2 %d %d %d %d ", net.w, net.l, net.h, 0);
	printFanout(out, fanout);
	int32_t nodeCount = net.w*net.l*net.h;
	int64_t edgeCountEst = (int64_t)fanout->count*(int64_t)nodeCount;
	printf("Fanout: %d\nNode count: %d\n", fanout->count, nodeCount);
	printf("Approx. edge count: %ld\nEst. file size: %.1lfMB\n", edgeCountEst, (double)edgeCountEst*2.5/(double)(1024*1024));
	
	int32_t r = fanout->r;
	printf("Generating edges...\n");
	Progress prog;
	start(&prog, net.h*net.w*net.l, 1000);
	fprintf(out, "BEGIN\n");
	for(int32_t sk=0; sk<net.h; sk++)
		for(int32_t sj=0; sj<net.l; sj++) {
			for(int32_t si=0; si<net.w; si++) {
				int32_t s = net.nodeIndex(si, sj, sk);
				float sx = net.itox(si);
				float sy = net.jtoy(sj);
				float sz = net.ktoz(sk);
				for(int32_t rk=-r; rk<=r; rk++)
					for(int32_t rj=-r; rj<=r; rj++)
						for(int32_t ri=-r; ri<=r; ri++) {
							if(fanout->gete(abs(ri), abs(rj), abs(rk))) {
								int32_t di = si+ri;
								int32_t dj = sj+rj;
								int32_t dk = sk+rk;
								if(di<0 || dj<0 || dk<0) // || di>=net.w || dj>=net.l || dk>=net.h)
									continue;
								int32_t d = net.nodeIndex(di, dj, dk);
								if(s<d) {
									float dx = net.itox(di);
									float dy = net.jtoy(dj);
									float dz = net.ktoz(dk);
									int32_t x = round(src->line(sx, sy, sz, dx, dy, dz)*1000.0);
									fprintf(out, "%d ", x);
								}
							}
						}
				fprintf(out, "\n");
				next(&prog);
			}
		}
	fprintf(out, "END\n");
	printf("\nDone\n");
}

#endif
