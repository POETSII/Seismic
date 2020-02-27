import sys
sys.path.append('bin')
from com.xrbpowered.imgpaths.vol import GenNoise3D
from com.xrbpowered.imgpaths.vol import SourceData3D
from com.xrbpowered.imgpaths.vol import ForwardStar3D
from com.xrbpowered.imgpaths.vol import GridNet3D
from com.xrbpowered.imgpaths import ShortestPath
from com.xrbpowered.imgpaths import Profile

noise = GenNoise3D(256, 256, 1).generate()
print('Generated')
noise.writeImageCuts('cut2d', 128, 128, 0)
noise.writeBytes('src2d.bin')

#src = SourceData3D.readBytes('src3d.bin', 0.25, 1);
#net = GridNet3D.create(src, 1, 4, ForwardStar3D(5, False))
#net.write('test3dg4r5.txt', 0)

#net = GridNet3D.read('test3dg8r3.txt')
#print('Net loaded')

#root = net.nodeIndex(net.w/2, net.l/2, 2)
#prof = Profile.start('ShortestPath.calculate')
#sum = ShortestPath(net).calculate(root).sum()
#time = prof.finish()
#print('Solved in %dms' % time)
#print('sum = %f' % sum)

#ForwardStar3D(7, False).print();

print('Done')
