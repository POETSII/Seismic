import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import Net
from com.xrbpowered.imgpaths import ShortestPath

#net = Net.random(1000, 5000, 1, 10)
#net.write('../data/inputs/w1k5k.txt', 3)
net = Net.read('../data/inputs/w1k5k.txt')

res = ShortestPath(net).calculate(0)

sum = 0.0
for node in res.nodes:
	sum += node.dist
	print('%d: %f' % (node.index, node.dist))

print('sum = %f' % sum)

print('Done')
