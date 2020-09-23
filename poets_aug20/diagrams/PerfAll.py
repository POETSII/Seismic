from jdiag import *

from com.xrbpowered.jdiagram.data import NumberFormatter
from com.xrbpowered.jdiagram.data import ExpFormatter

from com.xrbpowered.jdiagram.chart import Page
from com.xrbpowered.jdiagram.chart import ScatterChart
from com.xrbpowered.jdiagram.chart.ScatterChart import Population
from com.xrbpowered.jdiagram.chart import Anchor

plots = [
	# plot title, col, axis label, log axis, axis min, axis max, grid step, axis label offs, number format
	('Performance', 'ptime', 'compute time, s', True, 1.0e-3, 1.0e1, 10, -35, ExpFormatter.svgExp(10, 5))
#	('Time/step', 'steptime', 'step time, s', True, 1.0e-6, 1.0e0, 10, -40, ExpFormatter.svgExp(10, 5))
#	('Miss rate', 'miss%', 'miss rate, %', False, 0, 100, 10, -40, '%.0f%%')
#	('Off-chip memory', 'ocmem', 'GBytes/s', True, 1.0e-3, 1.0e2, 10, -40, ExpFormatter.svgExp(10, 5))
]

topologies = [
	('grid2d[4-con]', '4-connected 2D grid'),
	('grid3d[6-con]', '6-connected 3D grid'),
	('norm[k=8]', 'Random graph, |E| = 8&#x00b7;|V|'),
	('grid2d[8-con]', '8-connected 2D grid'),
	('grid3d[26-con]', '26-connected 3D grid'),
	('norm[k=16]', 'Random graph, |E| = 16&#x00b7;|V|')
	#('tree[2]', 'Binary tree'),
	#('ring', 'Ring')
]

# prefix, label, style
platforms = [
	# label, prefix, style
	('CPU-', 'cpu', 'stroke-width:0.5;stroke:#333'),
	('S', 'sync', 'stroke-width:2;stroke:#009'),
	('A', 'async', 'stroke-width:2;stroke:#f90')
]
costs = [
	# prefix, title, style
	('W', 'weights', ''),
	('H', 'no weights', 'stroke-dasharray: 2 3')
]

data = Data.read(File('data_ex.csv'))
data.append(Data.read(File('data_grid.csv')))

# add calculated cols
data.addCol('CPU-H:ptime', get('CPU-H:time'))
data.addCol('CPU-W:ptime', get('CPU-W:time'))
for (costPrefix, cost, costStyle) in costs:
	for (pf, pfLabel, pfStyle) in platforms:
		def pref(hdr):
			return '%s%s:%s' % (pf, costPrefix, hdr)
		data.addCol(pref('steptime'), tryOrNull(True, ratio(getNum(pref('ptime')), getNum('maxhops' if costPrefix=='H' else 'maxhopsW'))))
		data.addCol(pref('miss%'), tryOrNull(True, prod(getNum(pref('miss')), val(100.0))))

page = Page(3).setGap(5)

col = 0
row = 0
for (title, hdr, axisLabel, logAxis, axisMin, axisMax, axisStep, axisLabelOffs, numFmt) in plots:
	for (selectTopology, topName) in topologies:
		topsel = data.filter(Filter.filter('topology', selectTopology))
		
		# set up a new chart
		chart = ScatterChart().setSize(350, 200).setTitle(topName)
		chart.setMargins(50, 20, 40, 30) # 60 if row==1 else 10)
		chart.legend.setCols(6).posBottom(-40).setItemSize(120, 20)
		chart.clipChart = True
		
		# set up axes
		chart.axisx.setLogRange(1024, 1024*1024, 2) \
				.setAnchor(Anchor.bottom).setLabel('nodes').setNumberFormatter(ExpFormatter.svgExp(2, 5))
		chart.axisy.setRange(logAxis, axisMin, axisMax, axisStep) \
				.setAnchor(Anchor.left).setLabel(axisLabel, Anchor.left.offset(axisLabelOffs)).setNumberFormatter(numFmt)

		# add data lines
		for (costPrefix, cost, costStyle) in costs:
			for (pf, pfLabel, pfStyle) in platforms:
				phdr = '%s%s:%s' % (pf, costPrefix, hdr)
				pfsel = topsel.filter(Filter.notNull(['%s%s:time' % (pf, costPrefix), phdr]))
				chart.addPopLegend('%s, %s' % (pfLabel, cost),
					Population(pfsel, 'nodes', phdr, 'fill:none;%s;%s' % (pfStyle, costStyle))
				)
		page.add(chart)
		
		if col!=1 or row!=1: chart.legend.items.clear()
		col = col+1
		if col==3:
			col = 0
			row = row+1

# finish and print
page.printPage(System.out)
