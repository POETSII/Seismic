import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ImagePaths
from com.xrbpowered.imgpaths import GridNet
from com.xrbpowered.imgpaths import ForwardStar
from com.xrbpowered.imgpaths import ShortestPath
from com.xrbpowered.imgpaths import Solution
from com.xrbpowered.imgpaths import Profile

name = 'L4r7';
img = ImagePaths.readImage('../data/img/in3.png')

net = GridNet.read('../data/inputs/ints/%s.txt' % name)
print('Net loaded')

def show(res, type):
	for i in range(0, net.w, 1):
		res.markPath(i, net.h-1)
	ImagePaths.write(res, img, '../data/outputs/ints/%s_%s.png' % (name, type), False)


root = net.nodeIndex(net.w/2, 2)
res_pc = ShortestPath(net).calculate(root).solution()
print('pc sum = %f' % res_pc.sum())
show(res_pc, 'pc')

res_hw = Solution.read(net, '../data/outputs/ints/%s_hwi.txt' % name)
print('hw sum = %f' % res_hw.sum())
show(res_hw, 'hwi')

print('Done')
