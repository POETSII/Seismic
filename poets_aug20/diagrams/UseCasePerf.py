from jdiag import *

from com.xrbpowered.jdiagram.data import NumberFormatter
from com.xrbpowered.jdiagram.data import ExpFormatter

from com.xrbpowered.jdiagram.chart import Page
from com.xrbpowered.jdiagram.chart import ScatterChart
from com.xrbpowered.jdiagram.chart.ScatterChart import Population
from com.xrbpowered.jdiagram.chart import Anchor

platformsSeismic = [
	# name, label, style
	('oregon_2016_4N', '4 &#xd7; Intel Xeon X5650', 'stroke:#fb0'),
	('oregon_2018_1xV100', 'Nvidia Tesla V100', 'stroke:#5b0'),
	('oregon_2016_16N', '16 &#xd7; Intel Xeon X5650', 'stroke:#d70'),
	('poets_R1', 'POETS', 'stroke:#009')
]

platformsETX = [
	# name, label, style
	('single_PC', 'Single core Intel i5-6500', 'stroke:#fb0'),
	('GPU', 'Nvidia GeForce GTX 1050 Ti', 'stroke:#5b0'),
	('poets_sync', 'POETS sync', 'stroke:#009'),
	('poets_async', 'POETS async', 'stroke:#09d')
]

plots = [
	#('Platform Comparison: weighted SSSP', 'compute time, s', True, 1e5, 1e9, 10, ExpFormatter.svgExp(nodesStep, 5), True, 1.0e-2, 1.0e4, 10, -35, ExpFormatter.svgExp(10, 5))
	('Platform Comparison: unweighted APSP', 'compute time, s', False, 0, 20000, 5000, NumberFormatter.simple('%.0f'), True, 1.0e-2, 1.0e2, 10, -35, ExpFormatter.svgExp(10, 5))
]

#platforms = platformsSeismic
platforms = platformsETX

#data = Data.read(File('seismic_perf_data.csv'))
data = Data.read(File('etx_perf_data.csv'))

page = Page(1).setGap(5)

for (title, axisLabel, nodesLog, nodesMin, nodesMax, nodesStep, nodesFmt, logAxis, axisMin, axisMax, axisStep, axisLabelOffs, numFmt) in plots:
	#topsel = data.filter(Filter.filter('topology', selectTopology))
	
	# set up a new chart
	chart = ScatterChart().setSize(450, 200).setTitle(title)
	chart.setMargins(50, 20, 40, 80)
	chart.legend.setCols(2).posBottom(-40).setItemSize(180, 20)
	chart.clipChart = True
	
	# set up axes
	chart.axisx.setRange(nodesLog, nodesMin, nodesMax, nodesStep) \
			.setAnchor(Anchor.bottom).setLabel('nodes').setNumberFormatter(nodesFmt)
	chart.axisy.setRange(logAxis, axisMin, axisMax, axisStep) \
			.setAnchor(Anchor.left).setLabel(axisLabel, Anchor.left.offset(axisLabelOffs)).setNumberFormatter(numFmt)

	# add data lines
	for (pf, pfLabel, pfStyle) in platforms:
		pfsel = data.filter(Filter.filter('platform', pf))
		chart.addPopLegend(pfLabel,
			Population(pfsel, 'nodes', 'time', 'fill:none;stroke-width:2;%s' % pfStyle)
		)

	page.add(chart)

# finish and print
page.printPage(System.out)
