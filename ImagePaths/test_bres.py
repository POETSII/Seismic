import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ImagePaths
from com.xrbpowered.imgpaths import Fanout
from com.xrbpowered.imgpaths import ForwardStar
from com.xrbpowered.imgpaths import GridNet
from com.xrbpowered.imgpaths import ShortestPath
from com.xrbpowered.imgpaths import Solution

'''
def gen(x):
	img = ImagePaths.readImage('../data/img/f%d.png' % x)
	ImagePaths.writeImageBytes(img, '../data/img/f%d.bin' % x)
	print('Done f%d' % x)
for x in range(6):
	gen(x)
'''

'''
def gen(data, prefix, name, r, fanout):
	net = GridNet.create(data, 4, fanout)
	file = '%s_%s%d' % (prefix, name, r)
	net.write('../data/inputs/test_bres_py/%s.txt' % file, 0)
	print('Done %s' % file)

for prefix in [('f%d' % x) for x in range(6)]:
	data = ImagePaths.read('../data/img/%s.png' % prefix, 0.25, 1)
	gen(data, prefix, 'G', 0, Fanout.Full(23))
	for r in [2, 3, 5, 7]:
		gen(data, prefix, 'full', r, Fanout.Full(r))
		gen(data, prefix, 'fwds', r, ForwardStar(r))
		gen(data, prefix, 'bres', r, Fanout.Bresenham(r))
'''

gold = {}
#img = {}

def run(prefix, name, r, g=False):
	file = '%s_%s%d' % (prefix, name, r)
	net = GridNet.read('../data/inputs/test_bres_py/%s.txt' % file)

	root = net.nodeIndex(net.w/2, 2)
	res = ShortestPath(net).calculate(root).solution()
	sum = res.sum()
	if(g):
		gold[prefix] = sum
		#img[prefix] = ImagePaths.readImage('../data/img/%s.png' % prefix)
		err = 0
	else:
		err = (sum-gold[prefix]) / gold[prefix]
	#ImagePaths.write(res, img[prefix], '../data/inputs/test_bres/%s.png' % file, True)
	print('%s\t%s\t%d\t%d\t%.5f' % (prefix, name, r, sum, err))

for prefix in [('f%d' % x) for x in range(6)]:
	run(prefix, 'G', 0, True)
	for name in ['full', 'fwds', 'bres']:
		for r in [2, 3, 5, 7]:
			run(prefix, name, r)

print('Done')
