import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ImagePaths
from com.xrbpowered.imgpaths import SourceData

SourceData.lineStep = 1.0
prec = 1000.0
step = 1
nsteps = 800
xstart = 16
y = 64

def fixed_pt(x):
	return int(x*prec) if (prec>0) else x

data = ImagePaths.read('../data/img/in3.png', 0.25, 1)
sum = 0
xend = xstart+step*nsteps
for x in range(xstart, xend+step, step):
	s = fixed_pt(data.line(x, y+x, x+step, y+x+step))
	#print("%.3f" % s)
	sum += s
print("sum: %d" % sum)
single = fixed_pt(data.line(xstart, y+xstart, xend+step, y+xend+step))
print("single: %d" % single)
err = abs(sum-single)/float(single)*100.0
print("err=%.5f%%" % err)
