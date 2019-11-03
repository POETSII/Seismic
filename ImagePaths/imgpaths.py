import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ImagePaths
from com.xrbpowered.imgpaths import Net
from com.xrbpowered.imgpaths import ForwardStar
from com.xrbpowered.imgpaths import ShortestPath
from com.xrbpowered.imgpaths import Solution
from com.xrbpowered.imgpaths import Profile

#img = ImagePaths.readImage('in3.png')
#ImagePaths.writeImageBytes(img, 'in3.bin');

data = ImagePaths.read('../data/img/in3.png', 0.25, 1)
net = Net.create(data, 2, ForwardStar(3))
print('Net created')
#net.write('in3_g32_r3.txt', 3)
#print('Net saved')
#net.writeEdges('s3_edges.txt', 3)
#net = Net.read('../data/inputs/L4r5.txt')
#print('Net loaded')
#net = Net(256, 256, 4, None)

root = net.nodeIndex(net.w/2, 2)
prof = Profile.start('ShortestPath.calculate')
res = ShortestPath(net).calculate(root)
time = prof.finish()
print('Solved in %dms' % time)
#res = Solution.read(net, '../data/outputs/L4a5_hw.txt')

#for i in range(0, net.w, 1):
#	res.markPath(i, net.h-1)

#img = data.img
#img = ImagePaths.readImage('../data/img/in3.png')
#ImagePaths.write(res, img, '../data/outputs/L4f5_pc.png', False)
#res.write('lrg_out_java.txt', 3);

print('Done')
