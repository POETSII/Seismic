import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ForwardStar
from com.xrbpowered.imgpaths import Fanout

'''
print(ForwardStar.primes)

for r in range(1, 8):
	ForwardStar(r).print()
'''

'''
r = 7
print("Full")
Fanout.Full(r).printAngles()
print("FwdS")
ForwardStar(r).printAngles()
print("Bres")
Fanout.Bresenham(r).printAngles()
'''

print("r\tfanout\tangles\tmaxgap\tavggap")

def printFanoutInfo(r):
	f = ForwardStar(r)
	print("%d\t%d\t%d\t%.3f\t%.3f" % (r, f.getMaxFanout(), f.angles().size()*4, f.maxGap(), f.meanGap()))

for r in range(1, 16):
	printFanoutInfo(r)
