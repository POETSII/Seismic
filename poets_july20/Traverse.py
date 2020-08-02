from jdiag import *

from com.xrbpowered.jdiagram.chart import Page
from com.xrbpowered.jdiagram.chart import ScatterChart
from com.xrbpowered.jdiagram.chart.ScatterChart import Population
from com.xrbpowered.jdiagram.chart import Anchor

plots = [
	# has cpu data, plot title, col, axis label, log axis, axis min, axis max, grid step, axis label offs, number format
	(True, 'Performance', 'time', 'compute time, s', True, 1.0e-3, 1.0e3, 10, -60, '%.1e'),
	(True, 'Time/step', 'steptime', 'step time, s', True, 1.0e-8, 1.0e1, 10, -60, '%.1e'),
	(False, 'Msg density', 'msgden', 'messages per edge', False, 0, 25, 5, -30, '%.0f'),
	(False, 'Throughput per node', 'nodethr', 'messages / node / s', True, 1.0e-3, 1.0e3, 10, -60, '%.1e')
]

topologies = [
	# name, style
	('norm[k=16]', 'stroke:#009'),
	('norm[k=8]', 'stroke:#09d'),
	('ring', 'stroke:#d70'),
	('tree[2]', 'stroke:#590')
]
platforms = [
	# label, prefix, style
	('cpu', 'CPU-', 'stroke-dasharray:10 4 2 4'),
	('sync', 'S', 'stroke-dasharray:2 2'),
	('async', 'A', '')
]
costs = [
	# prefix, title
	('H', 'unweighted'),
	('W', 'weighted')
]

data = Data.read(File('traverse.csv'))
# add calculated cols
for (costPrefix, cost) in costs:
	for (pfLabel, pf, pfStyle) in platforms:
		def pref(hdr):
			return '%s%s:%s' % (pf, costPrefix, hdr)
		data.addCol(pref('steptime'), tryOrNull(True, ratio(getNum(pref('time')), getNum('maxhops' if costPrefix=='H' else 'maxhopsW'))))
		if pfLabel!='cpu':
			data.addCol(pref('msgden'), tryOrNull(True, ratio(getNum(pref('msg')), getNum('edges'))))
			data.addCol(pref('nodethr'), tryOrNull(True, ratio(ratio(getNum(pref('msg')), getNum('nodes')), getNum(pref('time')))))

page = Page(2).setGap(20)

for (hasCpu, title, hdr, axisLabel, logAxis, axisMin, axisMax, axisStep, axisLabelOffs, numFmt) in plots:
	for (costPrefix, cost) in costs:
		# set up a new chart
		chart = ScatterChart().setTitle('%s: %s SSSP' % (title, cost)).setSize(600, 400)
		chart.setMargins(80, 40, 40, 130)
		chart.legend.posBottom(-40).setCols(3 if hasCpu else 2).setItemSize(150, 20)
		chart.clipChart = True
		
		# set up axes
		chart.axisx.setLogRange(1024, 1024*1024, 2) \
				.setAnchor(Anchor.bottom).setLabel('nodes').setNumberFmt('%.0f')
		chart.axisy.setRange(logAxis, axisMin, axisMax, axisStep) \
				.setAnchor(Anchor.left).setLabel(axisLabel, Anchor.left.offset(axisLabelOffs)).setNumberFmt(numFmt)

		# add data lines
		for (top, topStyle) in topologies:
			sel = data.filter(Filter.filter('topology', top))
			for (pfLabel, pf, pfStyle) in platforms:
				if pfLabel!='cpu' or hasCpu:
					phdr = '%s%s:%s' % (pf, costPrefix, hdr)
					pfsel = sel.filter(Filter.notNull(['%s%s:time' % (pf, costPrefix), phdr]))
					chart.addPopLegend('%s %s' % (top, pfLabel),
						Population(pfsel, 'nodes', phdr, 'fill:none;stroke-width:1.5;%s;%s' % (topStyle, pfStyle))
					)
		page.add(chart)

# finish and print
page.printPage(System.out)
