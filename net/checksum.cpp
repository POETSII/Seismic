#include <string.h>
#include <unistd.h>

#include "Net.h"

const double prec = 1000.0;
const double step = 1;
const double nsteps = 800;
const double xstart = 16;
const double y = 64;

double fixed_pt(double x) {
	return (prec>0) ? floor(x*prec) : x;
}

int main(int argc, char *argv[])
{
	FILE* in = fopen(argv[1], "rb");
	SourceData data;
	readSourceData(in, &data, 0.25, 1);
	fclose(in);

	double sum = 0;
	double xend = xstart+step*nsteps;
	for(double x = xstart; x<xend+step; x+=step) {
		sum += fixed_pt(data.line(x, y+x, x+step, y+x+step));
	}
	printf("sum: %.0f\n", sum);
	double single = fixed_pt(data.line(xstart, y+xstart, xend+step, y+xend+step));
	printf("single: %.0f\n", single);
	double err = (sum-single)/(double)single*100.0;
	printf("err=%.5f%%\n", err);

	return 0;
}
