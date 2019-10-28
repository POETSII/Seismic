import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ForwardStar

print(ForwardStar.primes)

for r in range(1, 8):
	ForwardStar(r).print()
