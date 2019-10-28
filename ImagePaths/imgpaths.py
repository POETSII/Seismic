import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ImagePaths
from com.xrbpowered.imgpaths import Net
from com.xrbpowered.imgpaths import ForwardStar
from com.xrbpowered.imgpaths import ShortestPath
from com.xrbpowered.imgpaths import Solution

#img = ImagePaths.readImage('in3.png')
#ImagePaths.writeImageBytes(img, 'in3.bin');

#data = ImagePaths.read('in3.png', 0.25, 1)
#net = Net.create(data, 32, ForwardStar(3))
#print('Net created')
#net.write('in3_g32_r3.txt', 3)
#print('Net saved')
#net.writeEdges('s3_edges.txt', 3)
#net = Net.read('C:/Dev/VM/shared/poets/seismic/lrg_net.txt')
#print('Net loaded')
net = Net(256, 256, 4, None)

#root = net.nodeIndex(net.w/2, 32/32)
#res = ShortestPath(net).calculate(root)
#print('Solved')
#res = Solution.read(net, 'lrg_out_hw.txt')
res = Solution.read(net, 'L4r5_2.txt')

for i in range(0, net.w, 1):
	res.markPath(i, net.h-1)

#img = data.img
img = ImagePaths.readImage('in3.png')
ImagePaths.write(res, img, 'L4r5_hw2.png', False)
#res.write('lrg_out_java.txt', 3);

print('Done')
