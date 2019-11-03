import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ImagePaths
from com.xrbpowered.imgpaths import Net
from com.xrbpowered.imgpaths import ForwardStar
from com.xrbpowered.imgpaths import ShortestPath
from com.xrbpowered.imgpaths import Solution

net = Net.read('../data/inputs/L4r5.txt')
print('Net loaded')

root = net.nodeIndex(net.w/2, 2)
res = ShortestPath(net).calculate(root)
print('Solved')
res_hw = Solution.read(net, '../data/outputs/L4f5_sim.txt')

min = 10
max = -1
sum = 0
n = 0
for i in range(0, net.w*net.h):
	if(res.nodes[i].dist > 0):
		err = abs(res_hw.nodes[i].dist - res.nodes[i].dist) / res.nodes[i].dist
		if(err > max):
			max = err
		if(err < min):
			min = err
		sum = sum + err
		n = n + 1

print('Err: min=%f, max=%f, mean=%f' % (min, max, sum/n))
