import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ForwardStar
from com.xrbpowered.imgpaths import Fanout

'''
print(ForwardStar.primes)

for r in range(1, 8):
	ForwardStar(r).print()
'''

r = 7
print("Full")
Fanout.Full(r).printAngles()
print("FwdS")
ForwardStar(r).printAngles()
print("Bres")
Fanout.Bresenham(r).printAngles()
