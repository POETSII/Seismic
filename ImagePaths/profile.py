import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ImagePaths
from com.xrbpowered.imgpaths import Net
from com.xrbpowered.imgpaths import ForwardStar
from com.xrbpowered.imgpaths import ShortestPath
from com.xrbpowered.imgpaths import Solution
from com.xrbpowered.imgpaths import Profile

data = ImagePaths.read('../data/img/in3.png', 0.25, 1)
for step in [4]:
	for r in [3, 5, 7]:
		net = Net.create(data, step, ForwardStar(r))
		#net = Net.read('../data/inputs/L4r7.txt')
		root = net.nodeIndex(net.w/2, 2)
		prof = Profile.start('ShortestPath.calculate')
		res = ShortestPath(net).calculate(root)
		time = prof.finish()
		print('%d\t%d\t%d\t%f' % (step, r, time, res.sum()))
