from jdiag import *

from com.xrbpowered.jdiagram.chart import Page
from com.xrbpowered.jdiagram.chart import ScatterChart
from com.xrbpowered.jdiagram.chart.ScatterChart import Population
from com.xrbpowered.jdiagram.chart import Anchor

plots = [
	# has cpu data, plot title, col, axis label, log axis, axis min, axis max, grid step, axis label offs, number format
	('Performance', 'time', 'compute time, s', True, 1.0e-2, 1.0e3, 10, -60, '%.1e'),
	('POETS time', 'ptime', 'compute time (POETS), s', True, 1.0e-2, 1.0e3, 10, -60, '%.1e'),
	('Time/step', 'steptime', 'step time, s', True, 1.0e-6, 1.0e1, 10, -60, '%.1e'),
	('Msg density', 'msgden', 'messages per edge', False, 0, 25, 5, -30, '%.0f'),
	('Throughput', 'thr', 'messages / s', True, 1, 1.0e9, 10, -60, '%.1e'),
	('Throughput per node', 'nodethr', 'messages / node / s', True, 1.0e-3, 1.0e5, 10, -60, '%.1e'),
	('CPU utilisation', 'cpu%', 'cpu utisilation, %', False, 0, 100, 10, -40, '%.0f%%'),
	('Miss rate', 'miss%', 'miss rate, %', False, 0, 100, 10, -40, '%.0f%%'),
	('Off-chip memory', 'ocmem', 'GBytes/s', True, 1.0e-3, 1.0e2, 10, -60, '%.1e'),
	('Inter-board messages', 'msg-xb', 'messages', True, 1, 1.0e9, 10, -60, '%.1e'),
	('Messages (total)', 'msg', 'messages', True, 1, 1.0e9, 10, -60, '%.1e')
]

topologies = [
	# name, style
	#('norm[k=16]', 'stroke:#009'),
	#('norm[k=8]', 'stroke:#09d'),
	('ring', 'stroke:#d70')
	#('tree[2]', 'stroke:#590')
]
mappings = [
	# name, style
	('direct', 'stroke:#009'),
	('random', 'stroke:#d70'),
	('metis', 'stroke:#590')
]
# prefix, title
platforms = [ ('S', 'Sync'), ('A', 'Async') ]
costs = [ ('H', 'unweighted') ] #, ('W', 'weighted') ]

data = Data.read(File('data_map.csv'))
# add calculated cols
for (costPrefix, cost) in costs:
	for (pf, pfLabel) in platforms:
		def pref(hdr):
			return '%s%s:%s' % (pf, costPrefix, hdr)
		data.addCol(pref('steptime'), tryOrNull(True, ratio(getNum(pref('ptime')), getNum('maxhops' if costPrefix=='H' else 'maxhopsW'))))
		data.addCol(pref('msgden'), tryOrNull(True, ratio(getNum(pref('msg')), getNum('edges'))))
		data.addCol(pref('thr'), tryOrNull(True, ratio(getNum(pref('msg')), getNum(pref('ptime')))))
		data.addCol(pref('nodethr'), tryOrNull(True, ratio(ratio(getNum(pref('msg')), getNum('nodes')), getNum(pref('ptime')))))
		data.addCol(pref('cpu%'), tryOrNull(True, prod(getNum(pref('cpu')), val(100.0))))
		data.addCol(pref('miss%'), tryOrNull(True, prod(getNum(pref('miss')), val(100.0))))

page = Page(len(platforms)*len(costs)).setGap(20)

for (title, hdr, axisLabel, logAxis, axisMin, axisMax, axisStep, axisLabelOffs, numFmt) in plots:
	for (costPrefix, cost) in costs:
		for (pf, pfLabel) in platforms:
			# set up a new chart
			chart = ScatterChart().setTitle('%s: %s %s SSSP' % (title, cost, pfLabel)).setSize(600, 400)
			chart.setMargins(80, 40, 40, 80)
			chart.legend.posBottom(-40).setItemSize(120, 20).setCols(len(topologies)*len(mappings))
			chart.clipChart = True
			
			# set up axes
			chart.axisx.setLogRange(1024, 1024*1024, 2) \
					.setAnchor(Anchor.bottom).setLabel('nodes').setNumberFmt('%.0f')
			chart.axisy.setRange(logAxis, axisMin, axisMax, axisStep) \
					.setAnchor(Anchor.left).setLabel(axisLabel, Anchor.left.offset(axisLabelOffs)).setNumberFmt(numFmt)

			# add data lines
			for (top, topStyle) in topologies:
				for (map, mapStyle) in mappings:
					sel = data.filter(Filter.filter(['topology', 'mapping'], [top, map]))
					phdr = '%s%s:%s' % (pf, costPrefix, hdr)
					pfsel = sel.filter(Filter.notNull(['%s%s:time' % (pf, costPrefix), phdr]))
					chart.addPopLegend('%s %s' % (top, map),
						Population(pfsel, 'nodes', phdr, 'fill:none;stroke-width:1.5;%s' % mapStyle)
					)
			page.add(chart)

# finish and print
page.printPage(System.out)
