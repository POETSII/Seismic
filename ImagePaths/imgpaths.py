import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ImagePaths
from com.xrbpowered.imgpaths import SourceData
from com.xrbpowered.imgpaths import GridNet
from com.xrbpowered.imgpaths import ForwardStar
from com.xrbpowered.imgpaths import ShortestPath
from com.xrbpowered.imgpaths import Solution
from com.xrbpowered.imgpaths import Profile
'''
#img = ImagePaths.readImage('in3.png')
#ImagePaths.writeImageBytes(img, 'in3.bin');

#data = ImagePaths.read('../data/img/in3.png', 0.25, 1)
#net = GridNet.create(data, 2, ForwardStar(3))
#print('Net created')
#net.write('in3_g32_r3.txt', 3)
#print('Net saved')
#net.writeEdges('s3_edges.txt', 3)
net = GridNet.read('../data/inputs/ints/L4r7.txt')
print('Net loaded')
#net = GridNet(32, 32, 32, None)

root = net.nodeIndex(net.w/2, 2)
prof = Profile.start('ShortestPath.calculate')
res = ShortestPath(net).calculate(root).solution()
time = prof.finish()
print('Solved in %dms' % time)
#res = Solution.read(net, '../data/outputs/L32r7_hw.txt')
print('sum = %f' % res.sum())

#for i in range(0, net.w, 1):
	#print(res.nodes[net.nodeIndex(i, net.h-1)].dist)
	#res.markPath(i, net.h-1)

#img = data.img
#img = ImagePaths.readImage('../data/img/in3.png')
#ImagePaths.write(res, img, '../data/outputs/Loffs2r3_pc.png', False)
res.write('../data/outputs/ints/L4r7_pc.txt', 0);
'''

gold = 0
def run(file, g=False):
	global gold
	net = GridNet.read('../data/inputs/test_bres_py/%s.txt' % file)
	root = net.nodeIndex(net.w/2, 2)
	res = ShortestPath(net).calculate(root).solution()
	sum = res.sum()
	if(g):
		gold = sum
		err = 0
	else:
		err = (sum-gold) / gold
	print('%s\t%d\t%.5f' % (file, sum, err))
run('f3_G0', True)
run('f3_fwds5')
run('f3_fwds5_fb')
	
print('Done')
