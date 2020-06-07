import math

R = 5
print('const float precalcDist[%d] = {' % ((R+1)*(R+1)))
vals = ',\n\t'.join([', '.join([str(math.sqrt(x*x+y*y)) for x in range(6)]) for y in range(6)])
print('\t'+vals)
print('};')
