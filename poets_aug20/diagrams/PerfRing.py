from jdiag import *

from com.xrbpowered.jdiagram.data import NumberFormatter
from com.xrbpowered.jdiagram.data import ExpFormatter

from com.xrbpowered.jdiagram.chart import Page
from com.xrbpowered.jdiagram.chart import ScatterChart
from com.xrbpowered.jdiagram.chart.ScatterChart import Population
from com.xrbpowered.jdiagram.chart import Anchor

plots = [
	# has cpu data, plot title, col, axis label, log axis, axis min, axis max, grid step, axis label offs, number format
#	('Performance', 'time', 'compute time, s', True, 1.0e-2, 1.0e3, 10, -60, '%.1e'),
	('Performace', 'ptime', 'compute time, s', True, 1.0e-2, 1.0e3, 10, -35, ExpFormatter.svgExp(10, 5)),
	('Time/step', 'steptime', 'step time, s', True, 1.0e-6, 1.0e-2, 10, -40, ExpFormatter.svgExp(10, 5)),
#	('Msg density', 'msgden', 'messages per edge', False, 0, 25, 5, -30, '%.0f'),
#	('Throughput', 'thr', 'messages / s', True, 1, 1.0e9, 10, -60, '%.1e'),
#	('Throughput per node', 'nodethr', 'messages / node / s', True, 1.0e-3, 1.0e5, 10, -60, '%.1e'),
#	('CPU utilisation', 'cpu%', 'cpu utisilation, %', False, 0, 100, 10, -40, '%.0f%%'),
	('Cache miss rate', 'miss%', 'miss rate, %', False, 0, 100, 10, -40, NumberFormatter.simple('%.0f%%'))
#	('Off-chip memory', 'ocmem', 'GBytes/s', True, 1.0e-3, 1.0e2, 10, -60, '%.1e'),
#	('Intra-thread messages', 'msg-it', 'messages', True, 1, 1.0e9, 10, -60, '%.1e'),
#	('Inter-thread messages', 'msg-xt', 'messages', True, 1, 1.0e9, 10, -60, '%.1e'),
#	('Inter-board messages', 'msg-xb', 'messages', True, 1, 1.0e9, 10, -60, '%.1e'),
#	('Messages (total)', 'msg', 'messages', True, 1, 1.0e9, 10, -60, '%.1e')
]

topologies = [
	# name, label, style
	('ring', 'ring', 'stroke:#930'),
	('tree[2]', 'binary tree', 'stroke:#5b0')
]
# prefix, label, style
platforms = [
	# label, prefix, style
	('S', 'sync', ''),
	('A', 'async', 'stroke-dasharray: 2 3')
]

data = Data.read(File('data_ex.csv'))
# add calculated cols
for (pf, pfLabel, pfStyle) in platforms:
	def pref(hdr):
		return '%sH:%s' % (pf, hdr)
	data.addCol(pref('steptime'), tryOrNull(True, ratio(getNum(pref('ptime')), getNum('maxhops'))))
	data.addCol(pref('msgden'), tryOrNull(True, ratio(getNum(pref('msg')), getNum('edges'))))
	data.addCol(pref('thr'), tryOrNull(True, ratio(getNum(pref('msg')), getNum(pref('ptime')))))
	data.addCol(pref('nodethr'), tryOrNull(True, ratio(ratio(getNum(pref('msg')), getNum('nodes')), getNum(pref('ptime')))))
	data.addCol(pref('cpu%'), tryOrNull(True, prod(getNum(pref('cpu')), val(100.0))))
	data.addCol(pref('miss%'), tryOrNull(True, prod(getNum(pref('miss')), val(100.0))))

page = Page(3).setGap(5)

col = 0
for (title, hdr, axisLabel, logAxis, axisMin, axisMax, axisStep, axisLabelOffs, numFmt) in plots:
	# set up a new chart
	chart = ScatterChart().setTitle(title).setSize(350, 200)
	chart.setMargins(50, 20, 40, 60)
	chart.legend.setCols(4).posBottom(-40).setItemSize(120, 20)
	chart.clipChart = True
	
	# set up axes
	chart.axisx.setLogRange(1024, 1024*1024, 2) \
			.setAnchor(Anchor.bottom).setLabel('nodes').setNumberFormatter(ExpFormatter.svgExp(2, 5))
	chart.axisy.setRange(logAxis, axisMin, axisMax, axisStep) \
			.setAnchor(Anchor.left).setLabel(axisLabel, Anchor.left.offset(axisLabelOffs)).setNumberFormatter(numFmt)

	# add data lines
	for (pf, pfLabel, pfStyle) in platforms:
		for (top, topLabel, topStyle) in topologies:
			sel = data.filter(Filter.filter('topology', top))
			phdr = '%sH:%s' % (pf, hdr)
			pfsel = sel.filter(Filter.notNull(['%sH:time' % pf, phdr]))
			chart.addPopLegend('%s %s' % (topLabel, pfLabel),
				Population(pfsel, 'nodes', phdr, 'fill:none;stroke-width:2;%s;%s' % (topStyle, pfStyle))
			)
	page.add(chart)
	
	if col!=1: chart.legend.items.clear()
	col = col+1

# finish and print
page.printPage(System.out)
