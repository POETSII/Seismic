from jdiag import *

from com.xrbpowered.jdiagram.data import NumberFormatter

from com.xrbpowered.jdiagram.chart import Page
from com.xrbpowered.jdiagram.chart import ScatterChart
from com.xrbpowered.jdiagram.chart.ScatterChart import Population
from com.xrbpowered.jdiagram.chart import Anchor

plots = [
	# has cpu data, plot title, col, axis label, log axis, axis min, axis max, grid step, axis label offs, number format
	('Steps', 'step count', True, 1.0e0, 1.0e4, 10, -35, NumberFormatter.exp(10)),
]

topologies = [
	# name, label, include w, style
	('grid2d[4-con]', '4-con. 2D grid', True, 'stroke:#555'),
	('grid2d[8-con]', '8-con. 2D grid', True, 'stroke:#999'),
	('grid3d[6-con]', '6-con. 3D grid', True, 'stroke:#e90'),
	('grid3d[26-con]', '26-con. 3D grid', True, 'stroke:#fb5'),
	('norm[k=8]', 'random (k=8)', True, 'stroke:#09d'),
	('norm[k=16]', 'random (k=16)', True, 'stroke:#009'),
	('ring', 'ring', False, 'stroke:#930'),
	('tree[2]', 'binary tree', False, 'stroke:#5b0')
]
# prefix, label, style
weights = [
	# label, prefix, style
	('', 'no weights', ''),
	('W', 'weights', 'stroke-dasharray: 2 3')
]

data = Data.read(File('data_ex.csv'))
data.append(Data.read(File('data_grid.csv')))

page = Page(1).setGap(5)

for (title, axisLabel, logAxis, axisMin, axisMax, axisStep, axisLabelOffs, numFmt) in plots:
	# set up a new chart
	chart = ScatterChart().setTitle(title).setSize(450, 400)
	chart.setMargins(50, 20, 40, 180)
	chart.legend.setCols(2).posBottom(-40).setItemSize(200, 20)
	chart.clipChart = True
	
	# set up axes
	chart.axisx.setLogRange(1024, 1024*1024, 2) \
			.setAnchor(Anchor.bottom).setLabel('nodes').setNumberFormatter(NumberFormatter.exp(2))
	chart.axisy.setRange(logAxis, axisMin, axisMax, axisStep) \
			.setAnchor(Anchor.left).setLabel(axisLabel, Anchor.left.offset(axisLabelOffs)).setNumberFormatter(numFmt)

	# add data lines
	for (top, topLabel, useW, topStyle) in topologies:
		for (w, wLabel, wStyle) in weights:
			if useW or w!='W':
				sel = data.filter(Filter.filter('topology', top))
				phdr = 'maxhops%s' % w
				chart.addPopLegend('%s, %s' % (topLabel, wLabel) if useW else '%s, weights / no weights' % topLabel,
					Population(sel, 'nodes', phdr, 'fill:none;stroke-width:2;%s;%s' % (topStyle, wStyle))
				)
	page.add(chart)

# finish and print
page.printPage(System.out)
