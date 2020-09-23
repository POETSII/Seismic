from jdiag import *

from com.xrbpowered.jdiagram.data import ExpFormatter

from com.xrbpowered.jdiagram.chart import Page
from com.xrbpowered.jdiagram.chart import ScatterChart
from com.xrbpowered.jdiagram.chart.ScatterChart import Population
from com.xrbpowered.jdiagram.chart import Anchor

from com.xrbpowered.jdiagram.chart import StepRenderer

'''
resNames = [
	('result_g%d_AH', 'async, no weights, %d-con, bucket-fill', 'stroke:#5b0'),
	('result_g%d_AW', 'async, weights, %d-con, bucket-fill', 'stroke:#009'),
	('metis_g%d_AW', 'async, weights, %d-con, metis', 'stroke:#e90')
]
cons = [
	(4, 'stroke-width:1.5;stroke-dasharray: 2 2'),
	(8, 'stroke-width:1.5')
]
'''

data = Data.read(File('randgrid_dd.csv'))

page = Page(1).setGap(5)

for log in [False]:
	# set up a new chart
	chart = ScatterChart().setTitle('Degree Distribution').setSize(500, 300)
	chart.setMargins(65, 20, 40, 100)
	chart.legend.setCols(5).posBottom(-40).setItemSize(80, 20)
	chart.clipChart = True

	# set up axes
	chart.axisx.setRange(0, 35, 5) \
			.setAnchor(Anchor.bottom).setLabel('degree').setNumberFmt('%.0f')
	if not log:
		chart.axisy.setRange(False, 0, 3e5, 5e4) \
				.setAnchor(Anchor.left).setLabel('nodes', Anchor.left.offset(-50)).setNumberFmt('%.0f')
	else:
		chart.axisy.setRange(True, 1, 1024*1024, 4) \
				.setAnchor(Anchor.left).setLabel('nodes', Anchor.left.offset(-30)).setNumberFormatter(ExpFormatter.svgExp(2, 5))

	# add data lines
	for r in range(0, 110, 10):
		phdr = 'rgrid%d' % r
		chart.addPopLegend('r=%.1f' % (r/100.0),
			Population(data, 'degree', phdr, 'fill:none;stroke-width:1.5;stroke:#%x55' % (r/10+5)) #.setRenderer(StepRenderer())
		)
	page.add(chart)

# finish and print
page.printPage(System.out)
