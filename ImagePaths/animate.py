import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ImagePaths
from com.xrbpowered.imgpaths import GridNet
from com.xrbpowered.imgpaths import ForwardStar
from com.xrbpowered.imgpaths import ShortestPath
from com.xrbpowered.imgpaths import Solution
from com.xrbpowered.imgpaths import Profile

img = ImagePaths.readImage('../data/img/in3.png')
net = GridNet(256, 256, 4, None)
res = Solution.read(net, '../data/outputs/ints/L4r7_pc.txt')
print('Loaded')

#max = 0;
#for i in range(0, len(res.nodes)):
#	if(res.nodes[i].dist>max):
#		max = res.nodes[i].dist
#print('max = %f' % max)

# max = 688755.000000

step = 10000
span = 50000
for t in range(0, 800000, step):
	print(t)
	for i in range(0, len(res.nodes)):
		res.nodes[i].marked = res.nodes[i].dist<t and res.nodes[i].dist>=t-span
	#for i in range(0, len(res.nodes)):
	#	if(res.nodes[i].dist<t):
	#		res.markPath(i)
	ImagePaths.write(res, img, '../data/outputs/anim/res%03d.png' % (t/step), False)

print('Done')
