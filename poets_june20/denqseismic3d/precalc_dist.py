import math

R = 1
print('const float precalcDist[%d] = {' % ((R+1)*(R+1)*(R+1)))
vals = ',\n\t'.join([',\n\t'.join([', '.join([str(math.sqrt(x*x+y*y)) for x in range(R+1)]) for y in range(R+1)]) for z in range(R+1)])
print('\t'+vals)
print('};')
