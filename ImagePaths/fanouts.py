import sys
sys.path.append('imgpaths.jar')
from com.xrbpowered.imgpaths import ForwardStar
from com.xrbpowered.imgpaths import Fanout
from com.xrbpowered.imgpaths.vol import ForwardStar3D
from com.xrbpowered.imgpaths import CustomFanout

'''
for r in range(1, 15):
	ff = ForwardStar3D(r, False)
	fc = ForwardStar3D(r, True)
	print("%d\t%d\t%d" % (r, ff.getMaxFanout(), fc.getMaxFanout()))
'''

'''
print("r\tfanout\tangles\tmaxgap\tavggap")

def printFanoutInfo(f):
	print("%d\t%d\t%d\t%.3f\t%.3f" % (f.getRadius(), f.getMaxFanout(), f.angles().size()*4, f.maxGap(), f.meanGap()))

for r in range(1, 16):
	printFanoutInfo(ForwardStar(r))

fo = CustomFanout.readList('../data/818-FS.txt', 7)
fp = ForwardStar(7)
printFanoutInfo(fo)
printFanoutInfo(fp)

print(">>>> 818-FS.txt")
fo.print()
print(">>>> FS(7)")
fp.print()
'''

f = CustomFanout.readList('../data/818-FS.txt', 7)
#f = ForwardStar(7)

for gap in f.gaps():
	print("%.5f" % gap)
sys.exit()

r = 7
grid = 32
size = (3+r*2)*grid
print('<svg xmlns="http://www.w3.org/2000/svg" height="%d" width="%d">' % (size, size))

print('''<style>
	line { stroke:#000; stroke-width:2px}
	.coff { fill:#eee; stroke:#bbb; stroke-width:1}
	.con { fill:#000; stroke:#000; stroke-width:1}
	.cstart { fill:#e70; stroke:#a50; stroke-width:0.75; transform:scale(1.5, 1.5)}
</style>''')

print('<g transform="translate(%d %d)">' % (size/2, size/2))

for rx in range(r+1):
	for ry in range(r+1):
		cls = f.hasEdge(rx, ry)
		
		def line(cx, cy):
			if(cls): print('<line x1="%d" y1="%d" x2="%d" y2="%d" class="%s" />' % (cx*grid, cy*grid, -cx*grid, -cy*grid, cls))
		
		line(rx, ry)
		if(rx>0 and ry>0): line(-rx, -ry)
		if(rx>0): line(-rx, ry)
		if(ry>0): line(rx, -ry)

for rx in range(r+1):
	for ry in range(r+1):
		cls = 'coff'
		if(rx==0 and ry==0): cls = 'cstart'
		if(f.hasEdge(rx, ry)): cls = 'con'
		
		def circle(cx, cy):
			print('<circle cx="%d" cy="%d" r="3" class="%s" />' % (cx*grid, cy*grid, cls))
		
		circle(rx, ry)
		if(rx>0 and ry>0): circle(-rx, -ry)
		if(rx>0): circle(-rx, ry)
		if(ry>0): circle(rx, -ry)

print('</g></svg>')
